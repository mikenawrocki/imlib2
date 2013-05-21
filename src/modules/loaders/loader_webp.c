#include "loader_common.h"

#include <webp/encode.h>
#include <webp/decode.h>
#include <webp/types.h>

static void
comment_free(ImlibImage * im, void *data)
{
   free(data);
}

char
load(ImlibImage * im, ImlibProgressFunction progress,
     char progress_granularity, char immediate_load)
{
    FILE      *fp;
    size_t     len;
    uint8_t   *webp_data;
    struct WebPBitstreamFeatures features;

    /* Image has already been loaded, do not reload. */
    if(im->data)
        return 0;

    /* Unable to load the image file for reading, abort. */
    if(!(fp = fopen(im->real_file, "rb")))
        return 0;

    /* Determine file size, in bytes */
    fseek(fp, 0, SEEK_END);
    len = (size_t)ftell(fp);
    rewind(fp);

    /* Allocate a buffer the size of the file, read contents into memory */
    if(!(webp_data = malloc(len)) || 
        (len != fread(webp_data, sizeof(uint8_t), len, fp))) {
        fclose(fp);
        return 0;
    }

    if(WebPGetFeatures(webp_data, len, &features) != VP8_STATUS_OK)
        return 0;

    /* Update ImlibImage information */
    im->format = strdup("webp");
    im->h = features.height;
    im->w = features.width;

    im->flags = (features.has_alpha) ? 
        im->flags | F_HAS_ALPHA :
        im->flags & ~(F_HAS_ALPHA);

    if(im->loader || progress || immediate_load)
    {
        im->data = (DATA32 *)WebPDecodeBGRA(webp_data, len, 
                &features.width, &features.height);
        if(progress)
            progress(im, 100, 0,0,0,0);
    }

    free(webp_data);
    fclose(fp);
    return 1;
}

char
save(ImlibImage * im, ImlibProgressFunction progress, char progress_granularity)
{

}

void
formats(ImlibLoader * l)
{
   char               *list_formats[] = { "webp" };
   {
      int                 i;

      l->num_formats = (sizeof(list_formats) / sizeof(char *));
      l->formats = malloc(sizeof(char *) * l->num_formats);

      for (i = 0; i < l->num_formats; i++)
         l->formats[i] = strdup(list_formats[i]);
   }
}
// vim: ts=8 sw=4 sts=4 et
