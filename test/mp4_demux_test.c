/**
 * @file mp4_demux_test.c
 * @brief MP4 file library - demuxer test program
 * @date 07/11/2016
 * @author aurelien.barre@akaaba.net
 *
 * Copyright (c) 2016 Aurelien Barre <aurelien.barre@akaaba.net>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *   * Neither the name of the copyright holder nor the names of the
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <libmp4.h>


static const char *video_codec_type[MP4_VIDEO_CODEC_MAX] =
{
    "unknown",
    "H.264",
};


static const char *audio_codec_type[MP4_AUDIO_CODEC_MAX] =
{
    "unknown",
    "AAC",
};


static const char *metadata_value_name[MP4_METADATA_VALUE_TYPE_MAX] =
{
    "artist",
    "title",
    "date",
    "location",
    "comment",
    "copyright",
    "maker",
    "model",
    "version",
    "encoder",
};


static const char *cover_type[MP4_METADATA_COVER_TYPE_MAX] =
{
    "JPEG",
    "PNG",
    "BMP",
};


static void mp4_demux_print_tracks(struct mp4_demux *demux)
{
    mp4_track_info_t tk;
    int i, count, ret;

    count = mp4_demux_get_track_count(demux);

    for (i = 0; i < count; i++)
    {
        ret = mp4_demux_get_track_info(demux, i, &tk);
        if (ret == 0)
        {
            printf("Track #%d ID=%d\n", i, tk.id);
            switch (tk.type)
            {
                case MP4_TRACK_TYPE_VIDEO:
                    printf("  type: video\n");
                    printf("  codec: %s\n", video_codec_type[tk.video_codec]);
                    printf("  dimensions=%" PRIu32 "x%" PRIu32 "\n",
                           tk.video_width, tk.video_height);
                    if (tk.has_metadata)
                    {
                        printf("  metadata: present\n");
                        printf("  metadata content encoding: %s\n", tk.metadata_content_encoding);
                        printf("  metadata mime format: %s\n", tk.metadata_mime_format);
                    }
                    break;
                case MP4_TRACK_TYPE_AUDIO:
                    printf("  type: audio\n");
                    printf("  codec: %s\n", audio_codec_type[tk.audio_codec]);
                    printf("  channels: %" PRIu32 "\n", tk.audio_channel_count);
                    printf("  samples: %" PRIu32 "bit @ %.2fkHz\n",
                           tk.audio_sample_size, tk.audio_sample_rate / 1000.);
                    break;
                case MP4_TRACK_TYPE_HINT:
                    printf("  type: hint\n");
                    break;
                case MP4_TRACK_TYPE_METADATA:
                    printf("  type: metadata\n");
                    printf("  content encoding: %s\n", tk.metadata_content_encoding);
                    printf("  mime format: %s\n", tk.metadata_mime_format);
                    break;
                case MP4_TRACK_TYPE_TEXT:
                    printf("  type: text\n");
                    break;
                case MP4_TRACK_TYPE_CHAPTERS:
                    printf("  type: chapters\n");
                    break;
                default:
                    printf("  type: unknown\n");
                    break;
            }
            unsigned int hrs = (unsigned int)((tk.duration + 500000) / 1000000 / 60 / 60);
            unsigned int min = (unsigned int)((tk.duration + 500000) / 1000000 / 60 - hrs * 60);
            unsigned int sec = (unsigned int)((tk.duration + 500000) / 1000000 - hrs * 60 * 60 - min * 60);
            printf("  duration=%" PRIu64 " (%02d:%02d:%02d)\n", tk.duration, hrs, min, sec);
            printf("\n");
        }
    }
}


static void mp4_demux_print_metadata(struct mp4_demux *demux)
{
    char **values = NULL;

    int ret = mp4_demux_get_metadata_values(demux, &values);
    if (ret == 0)
    {
        printf("Metadata\n");
        int i;
        for (i = 0; i < MP4_METADATA_VALUE_TYPE_MAX; i++)
        {
            if (values[i])
            {
                printf("    %s: %s\n", metadata_value_name[i], values[i]);
            }
        }
        printf("\n");
    }

    uint8_t *cover_buffer = NULL;
    unsigned int cover_buffer_size = 0, cover_size = 0;
    mp4_metadata_cover_type_t type;
    ret = mp4_demux_get_metadata_cover(demux, cover_buffer, cover_buffer_size, &cover_size, &type);
    if ((ret == 0) && (cover_size > 0))
    {
        cover_buffer_size = cover_size;
        cover_buffer = malloc(cover_buffer_size);
        if (!cover_buffer)
        {
            return;
        }
        ret = mp4_demux_get_metadata_cover(demux, cover_buffer, cover_buffer_size, &cover_size, &type);
        if (ret == 0)
        {
            printf("Cover present (%s)\n", cover_type[type]);
#if 0
            FILE *fCover = fopen("cover.jpg", "wb");
            if (fCover)
            {
                fwrite(cover_buffer, cover_size, 1, fCover);
                fclose(fCover);
            }
#endif
        }
    }
}


static void mp4_demux_print_chapters(struct mp4_demux *demux)
{
    unsigned int chaptersCount = 0;
    uint64_t *chaptersTime = NULL;
    char **chaptersName = NULL;

    int ret = mp4_demux_get_chapters(demux, &chaptersCount, &chaptersTime, &chaptersName);
    if (ret == 0)
    {
        printf("Chapters\n");
        unsigned int i;
        for (i = 0; i < chaptersCount; i++)
        {
            printf("    Chapter #%d time=%" PRIu64 " '%s'\n",
                   i + 1, chaptersTime[i], chaptersName[i]);
        }
        printf("\n");
    }
}


static void mp4_demux_print_frames(struct mp4_demux *demux)
{
    mp4_track_info_t tk;
    mp4_track_sample_t sample;
    int i, count, ret, found = 0;
    unsigned int id;

    count = mp4_demux_get_track_count(demux);

    for (i = 0; i < count; i++)
    {
        ret = mp4_demux_get_track_info(demux, i, &tk);
        if ((ret == 0) && (tk.type == MP4_TRACK_TYPE_VIDEO))
        {
            id = tk.id;
            found = 1;
            break;
        }
    }

    if (!found)
    {
        return;
    }

    i = 0;
    do
    {
        ret = mp4_demux_get_track_next_sample(demux, id, NULL, 0, NULL, 0, &sample);
        if (ret == 0)
        {
            printf("Frame #%d size=%06" PRIu32 " metadata_size=%" PRIu32 " dts=%" PRIu64 " next_dts=%" PRIu64 "\n",
                   i, sample.sample_size, sample.metadata_size, sample.sample_dts, sample.next_sample_dts);
        }
        i++;
    }
    while (sample.sample_size);

    printf("\n");
}


int main(int argc, char **argv)
{
    int ret = 0;
    struct mp4_demux *demux;

    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(-1);
    }

    demux = mp4_demux_open(argv[1]);
    if (demux == NULL)
    {
        fprintf(stderr, "mp4_demux_open() failed\n");
        ret = -1;
    }
    else
    {
        mp4_demux_print_tracks(demux);
        mp4_demux_print_metadata(demux);
        mp4_demux_print_chapters(demux);
#if 0
        mp4_demux_print_frames(demux);
#endif
    }

    if (demux)
    {
        ret = mp4_demux_close(demux);
        if (ret < 0)
        {
            fprintf(stderr, "mp4_demux_close() failed\n");
            ret = -1;
        }
    }

    exit(ret);
}
