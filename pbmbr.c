#include <stdio.h>
#include <locale.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

static inline int pixel_lit(unsigned char *pixels, int width, int height, int x, int y)
{
    if (x < 0 || x >= width)
    {
        return 0;
    }
    else if (y < 0 || y >= height)
    {
        return 0;
    }
    else
    {
        int pixel_offset = width * y + x;

        return !!(pixels[pixel_offset / 8] & 1 << (pixel_offset % 8));
    }
}

int main(int argc, char *argv[])
{
    int type = 0;
    int width = 0;
    int height = 0;

    /* Read the header. */
    if (1 != scanf("P%d", &type) || 1 != scanf("%d", &width) || 1 != scanf("%d", &height))
    {
        fprintf(stderr, "header error, see https://en.wikipedia.org/wiki/Netpbm\n");
        exit(EXIT_FAILURE);
    }

    int total_pixels = width * height;
    unsigned char *pixels = calloc((total_pixels + 7) / 8, 1);
    int pixel_offset = 0;
    int value = 0;

    /* Now read the remaining bits and store them in the allocated area. */
    while ((value = getchar()) != EOF)
    {
        switch (value)
        {
            case '0':
            case '1':
                if (pixel_offset >= total_pixels)
                {
                    fprintf(stderr, "excess pixels, see https://en.wikipedia.org/wiki/Netpbm\n");
                }
                pixels[pixel_offset / 8] |= (value == '1') << (pixel_offset % 8);
                ++pixel_offset;
                break;
            default:
                break;
        }
    }

    /* Any command line arguments will make the output display in hashes and spaces, this is one pixel per cell on the screen. */
    if (argc > 1)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                printf("%c", pixel_lit(pixels, width, height, x, y) ? '#' : ' ');
            }
            printf("\n");
        }
    }
    else
    {
        /* Try rendering in Braille characters, that way we fit eight pixels into a character cell. */
        enum
        {
            CELL_WIDTH = 2,
            CELL_HEIGHT = 4,
        };
        int horizontal_cells = (width + CELL_WIDTH - 1) / CELL_WIDTH;
        int vertical_cells = (height + CELL_HEIGHT - 1) / CELL_HEIGHT;

        for (int vertical_cell = 0; vertical_cell < vertical_cells; ++vertical_cell)
        {
            for (int horizontal_cell = 0; horizontal_cell < horizontal_cells; ++horizontal_cell)
            {
                int top_left_x = horizontal_cell * CELL_WIDTH;
                int top_left_y = vertical_cell * CELL_HEIGHT;

                struct
                {
                    int x;
                    int y;
                } offset[] = {
                    {0, 0},
                    {0, 1},
                    {0, 2},
                    {0, 3},
                    {1, 0},
                    {1, 1},
                    {1, 2},
                    {1, 3},
                };

                unsigned int cell = 0;
                for (unsigned int bit = 0; bit < sizeof(offset) / sizeof(offset[0]); ++bit)
                {
                    cell |= pixel_lit(pixels, width, height, top_left_x + offset[bit].x, top_left_y + offset[bit].y) << bit;
                }

                uint8_t mapped_cell = 0; /* See https://en.wikipedia.org/wiki/Braille_Patterns */

                mapped_cell |= cell >> 0 & 0x07;
                mapped_cell |= cell >> 1 & 0x38;
                mapped_cell |= cell << 3 & 0x40;
                mapped_cell |= cell << 0 & 0x80;

                //
                // Reverse engineered the coding like this:
                //     for val in $(seq 0 255) ; do printf "%02x\n" $(( 0x2800 + $val)) ; done | sed "s/.*/echo $\'\\\U&\' '           '/" | bash | od -tx1
                //
                printf("%c%c%c", 0xe2, 0xa0 + (mapped_cell >> 6), 0x80 + (mapped_cell & 0x3f));
            }
            printf("\n");
        }
    }
}
