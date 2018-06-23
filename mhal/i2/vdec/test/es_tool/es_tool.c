#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef enum {
    E_CODEC_NONE,
    E_CODEC_H264,
    E_CODEC_H265,
    E_CODEC_MAX
} E_CODEC_TYPE;

int get_file_size(const char *file_path)
{
    struct stat st;

    if (stat(file_path, &st) == 0)
        return st.st_size;

    return -1;
}

void display_help(void)
{
    printf("\n");
    printf("Usage: es_tool [-h] [-i input_file] [-o output_file] [-c h264|h265]\n");
    printf("\th: display help\n");
    printf("\ti: setup input file path\n");
    printf("\to: setup output file path\n");
    printf("\tc: setup codec type, if not specify codec type, each nal will be one es pack\n");
}

char *find_start_code(char *input_buffer, int input_size, E_CODEC_TYPE codec_type)
{
    char *start_code_ptr = input_buffer;
    char return_next_nal = 0;

    if (codec_type == E_CODEC_NONE)
        start_code_ptr += 4;

    while (start_code_ptr - input_buffer < input_size)
    {
        if (start_code_ptr[0] == 0x00 &&
            start_code_ptr[1] == 0x00 &&
            start_code_ptr[2] == 0x00 &&
            start_code_ptr[3] == 0x01)
        {
            if (return_next_nal)
                return start_code_ptr;

            if (codec_type == E_CODEC_H264)
            {
                if ((start_code_ptr[4] & 0xF) == 0x1 || (start_code_ptr[4] & 0xF) == 0x5)
                    return_next_nal = 1;

                start_code_ptr++;
            }
            else if (codec_type == E_CODEC_H265)
            {
                if (start_code_ptr[4] == 0x02 || start_code_ptr[4] == 0x26)
                    return_next_nal = 1;

                start_code_ptr++;
            }
            else
                return start_code_ptr;
        }
        else
            start_code_ptr++;
    }

    return NULL;
}

void add_es_header(char *input_buffer, int input_size, char *output_file_path, E_CODEC_TYPE codec_type)
{
    int es_packet_size;
    char es_header[16] = {0}, *ptr = input_buffer, *start_code_ptr;
    int output_fd = -1;

    es_header[0] = 0x01;

    if ((output_fd = open(output_file_path, O_WRONLY | O_CREAT, 0)) < 0)
    {
        printf("open %s fail!\n", output_file_path);
        return;
    }

    while (ptr - input_buffer < input_size)
    {
        if (start_code_ptr = find_start_code(ptr, (input_size - (ptr - input_buffer)), codec_type))
            es_packet_size = start_code_ptr - ptr;
        else
            es_packet_size = input_size - (ptr - input_buffer);

        es_header[4] = (es_packet_size & 0xFF000000) >> 24;
        es_header[5] = (es_packet_size & 0x00FF0000) >> 16;
        es_header[6] = (es_packet_size & 0x0000FF00) >> 8;
        es_header[7] = (es_packet_size & 0x000000FF);
        write(output_fd, es_header, 16);
        write(output_fd, ptr, es_packet_size);
        //printf("es_packet_size = %d [0x%x]\n", es_packet_size, ptr[4]);
        ptr += es_packet_size;
    }
ADD_ES_EXIT:
    if (output_fd >= 0)
        close(output_fd);
}

void remove_es_header(char *input_buffer, int input_size, char *output_file_path)
{
    int es_packet_size;
    char *ptr = (char *)input_buffer;
    int output_fd = -1;

    if ((output_fd = open(output_file_path, O_WRONLY | O_CREAT, 0)) < 0)
    {
        printf("open %s fail!\n", output_file_path);
        return;
    }

    while (ptr - input_buffer < input_size)
    {
        es_packet_size = (ptr[4]<<24 & 0xFF000000)|(ptr[5]<<16 & 0x00FF0000)|(ptr[6]<<8 & 0x0000FF00)|(ptr[7] & 0x000000FF);
        if (write(output_fd, ptr + 16, es_packet_size) != es_packet_size)
        {
            printf("write %s fail!\n", output_file_path);
            goto REMOVE_ES_EXIT;
        }

        ptr += (16 + es_packet_size);
    }

REMOVE_ES_EXIT:
    if (output_fd >= 0)
        close(output_fd);
}

int main(int argc, char *argv[])
{
    int c, input_file_size, input_fd=-1;
    char *input_file_path=NULL, *output_file_path=NULL;
    char *input_buffer;
    E_CODEC_TYPE codec_type=E_CODEC_NONE;

    while ((c = getopt (argc, argv, "hi:o:c:")) != -1)
    {
        switch (c)
        {
            case 'i':
                input_file_path = optarg;
                break;
            case 'o':
                output_file_path = optarg;
                break;
            case 'c':
                if (!strncmp(optarg, "h264", sizeof("h264")))
                {
                    codec_type = E_CODEC_H264;
                    //printf("Codec is H.264\n");
                }
                else if (!strncmp(optarg, "h265", sizeof("h265")))
                {
                    codec_type = E_CODEC_H265;
                    //printf("Codec is H.265\n");
                }
                break;
            case 'h':
            default:
                display_help();
                return -1;
        }
    }

    if (input_file_path == NULL)
    {
        printf("Please specify input file name with -i\n");
        return -1;
    }

    if (output_file_path == NULL)
    {
        printf("Please specify output file name with -o\n");
        return -1;
    }

    if ((input_file_size = get_file_size(input_file_path)) > 0)
    {
        if (input_buffer = malloc(input_file_size))
        {
            if ((input_fd = open(input_file_path, O_RDONLY, 0)) >= 0)
            {
                if (read(input_fd, input_buffer, input_file_size) == input_file_size)
                {
                    if (input_buffer[0] == 0x00 &&
                        input_buffer[1] == 0x00 &&
                        input_buffer[2] == 0x00 &&
                        input_buffer[3] == 0x01)
                        add_es_header(input_buffer, input_file_size, output_file_path, codec_type);
                    else
                        remove_es_header(input_buffer, input_file_size, output_file_path);
                }
                else
                {
                    printf("read %s fail!\n", input_file_path);
                    goto ES_EXIT;
                }
            }
            else
            {
                printf("open %s fail!(ret = %d)\n", input_file_path, input_fd);
                goto ES_EXIT;
            }
        }
        else
        {
            printf("malloc memory fail(size = %d)\n", input_file_size);
            goto ES_EXIT;
        }
    }
    else
        printf("get file size fail!\n");

ES_EXIT:
    if (input_buffer)
        free(input_buffer);
    if (input_fd >= 0)
        close(input_fd);

	return 0;
}
