#include <assert.h>
#include "png.h"
#include "rwpng.h"
#include "cpe/pal/pal_unistd.h" /*for getpid()*/
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/file.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/cache/ui_cache_pixel_format.h"
#include "convert_ctx.h"

static int convert_save_res_read_image(
    convert_ctx_t ctx, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf, mainprog_info *mainprog_ptr);
static int convert_save_res_save_image(
    convert_ctx_t ctx, vfs_file_t output, mainprog_info *mainprog_ptr, int floyd, int force, int reqcolors);
static void convert_save_res_clear_image_info(convert_ctx_t ctx, mainprog_info *mainprog_ptr);

int convert_save_res_pngquant(convert_ctx_t ctx, vfs_file_t output, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf) {
    static mainprog_info rwpng_info;

    if (convert_save_res_read_image(ctx, res, res_buf, &rwpng_info) != 0) return -1;

    if (convert_save_res_save_image(ctx, output, &rwpng_info, /*floyd*/ 0, /*force*/ 0, /*reqcolors*/256) != 0) {
        CPE_ERROR(ctx->m_em, "convert res: %s: write fail!", ui_cache_res_path(res));
        convert_save_res_clear_image_info(ctx, &rwpng_info);
        return -1;
    }

    convert_save_res_clear_image_info(ctx, &rwpng_info);
    return 0;
}

static int convert_save_res_read_image(
    convert_ctx_t ctx, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf, mainprog_info *mainprog_ptr)
{
    png_uint_32  i, rowbytes;
    ui_cache_pixel_format_t format = ui_cache_texture_format(res);
    uch* rgba_data;
    
    bzero(mainprog_ptr, sizeof(*mainprog_ptr));
    
    /* setjmp() must be called in every function that calls a PNG-writing
     * libpng function, unless an alternate error handler was installed--
     * but compatible error handlers must either use longjmp() themselves
     * (as in this program) or exit immediately, so here we go: */

    if (setjmp(mainprog_ptr->jmpbuf)) {
        return -1;
    }
    
    mainprog_ptr->width = ui_cache_texture_width(res);
    mainprog_ptr->height = ui_cache_texture_height(res);
    mainprog_ptr->interlaced = 0;

    if (format != ui_cache_pf_r8g8b8a8) {
        CPE_ERROR(
            ctx->m_em, "convert_save_read_image: image %s format %d not support",
            ui_cache_res_path(res), ui_cache_texture_format(res));
        return -1;
    }

    mainprog_ptr->gamma = 0.0;
    //png_get_gAMA(png_ptr, info_ptr, &mainprog_ptr->gamma);
    mainprog_ptr->rowbytes = rowbytes =
        ui_cache_pixel_format_to_stride(format) * ui_cache_texture_width(res);
    mainprog_ptr->channels = 1;

    rgba_data = (uch*)ui_cache_pixel_buf_pixel_buf(res_buf);
    if ((mainprog_ptr->row_pointers = (png_bytepp)malloc(mainprog_ptr->height*sizeof(png_bytep))) == NULL) {
        CPE_ERROR(
            ctx->m_em, "convert_save_read_image: image %s unable to allocate row pointers",
            ui_cache_res_path(res));
        return -1;
    }

    /* CPE_INFO( */
    /*     ctx->m_em, "convert_save_read_image: channels = %d, rowbytes = %ld, height = %ld", */
    /*     mainprog_ptr->channels, rowbytes, mainprog_ptr->height); */

    for (i = 0;  i < mainprog_ptr->height;  ++i) {
        mainprog_ptr->row_pointers[i] = rgba_data + i*rowbytes;
    }

    mainprog_ptr->png_ptr = NULL;
    mainprog_ptr->info_ptr = NULL;
    mainprog_ptr->retval = 0;
    return 0;
}

typedef uch pixval;	/* GRR: hardcoded for now; later add 16-bit support */

typedef struct {
    uch r, g, b, a;
} apixel;

#define PAM_GETR(p) ((p).r)
#define PAM_GETG(p) ((p).g)
#define PAM_GETB(p) ((p).b)
#define PAM_GETA(p) ((p).a)
#define PAM_ASSIGN(p,red,grn,blu,alf) \
   do { (p).r = (red); (p).g = (grn); (p).b = (blu); (p).a = (alf); } while (0)
#define PAM_EQUAL(p,q) \
   ((p).r == (q).r && (p).g == (q).g && (p).b == (q).b && (p).a == (q).a)
#define PAM_DEPTH(newp,p,oldmaxval,newmaxval) \
   PAM_ASSIGN( (newp), \
      ( (int) PAM_GETR(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
      ( (int) PAM_GETG(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
      ( (int) PAM_GETB(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
      ( (int) PAM_GETA(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval) )

typedef struct acolorhist_item *acolorhist_vector;
struct acolorhist_item {
    apixel acolor;
    int value;
};

typedef struct acolorhist_list_item *acolorhist_list;
struct acolorhist_list_item {
    struct acolorhist_item ch;
    acolorhist_list next;
};

typedef acolorhist_list *acolorhash_table;

typedef struct box *box_vector;
struct box {
    int ind;
    int colors;
    int sum;
};

static acolorhist_vector mediancut(acolorhist_vector achv, int colors, int sum, pixval maxval, int newcolors);
static int redcompare (const void *ch1, const void *ch2);
static int greencompare (const void *ch1, const void *ch2);
static int bluecompare (const void *ch1, const void *ch2);
static int alphacompare (const void *ch1, const void *ch2);
static int sumcompare (const void *b1, const void *b2);
static acolorhist_vector pam_acolorhashtoacolorhist(acolorhash_table acht, int maxacolors);
static acolorhist_vector pam_computeacolorhist(apixel **apixels, int cols, int rows, int maxacolors, int* acolorsP);
static acolorhash_table pam_computeacolorhash(apixel** apixels, int cols, int rows, int maxacolors, int* acolorsP);
static acolorhash_table pam_allocacolorhash(void);
static int pam_addtoacolorhash(acolorhash_table acht, apixel *acolorP, int value);
static int pam_lookupacolor(acolorhash_table acht, apixel* acolorP);
static void pam_freeacolorhist(acolorhist_vector achv);
static void pam_freeacolorhash(acolorhash_table acht);
static char *pm_allocrow (int cols, int size);

#define MAXCOLORS  32767
#define FS_SCALE   1024     /* Floyd-Steinberg scaling factor */

#define LARGE_NORM
/* #define LARGE_LUM */   /* GRR 19970727:  this isn't well-defined for RGBA */

static int convert_save_res_save_image(
    convert_ctx_t ctx, vfs_file_t output, mainprog_info * rwpng_info, int floyd, int force, int reqcolors)
{
    apixel **apixels;
    register apixel *pP;
    register int col, limitcol;
    register int ind;
    uch *pQ, *outrow, **row_pointers=NULL;
    ulg rows, cols;
    pixval maxval, newmaxval;
    acolorhist_vector achv, acolormap=NULL;
    acolorhash_table acht;
    long *thisrerr = NULL;
    long *nextrerr = NULL;
    long *thisgerr = NULL;
    long *nextgerr = NULL;
    long *thisberr = NULL;
    long *nextberr = NULL;
    long *thisaerr = NULL;
    long *nextaerr = NULL;
    long *temperr;
    register long sr=0, sg=0, sb=0, sa=0, err;
    int row;
    int colors;
    int newcolors = 0;
    int usehash;
    int fs_direction = 0;
    int x;
    int bot_idx, top_idx;
    int remap[256];

    /* NOTE:  rgba_data and row_pointers are allocated but not freed in
     *        rwpng_read_image() */
    apixels = (apixel **)rwpng_info->row_pointers;
    assert(apixels);
    
    cols = rwpng_info->width;
    rows = rwpng_info->height;
    /* channels = rwpng_info->channels; */
    maxval = 255;	/* GRR TO DO:  allow either 8 or 16 bps */


    /*
    ** Step 2: attempt to make a histogram of the colors, unclustered.
    ** If at first we don't succeed, lower maxval to increase color
    ** coherence and try again.  This will eventually terminate, with
    ** maxval at worst 15, since 32^3 is approximately MAXCOLORS.
    [GRR POSSIBLE BUG:  what about 32^4 ?]
    */
    do {
        achv = pam_computeacolorhist(apixels, cols, rows, MAXCOLORS, &colors);
        if (achv != NULL) break;

        newmaxval = maxval / 2;
        for (row = 0; (ulg)row < rows; ++row) {
            for (col = 0, pP = apixels[row]; (ulg)col < cols; ++col, ++pP) {
                PAM_DEPTH( *pP, *pP, maxval, newmaxval );
            }
        }
        
        maxval = newmaxval;
    } while(1);
    newcolors = MIN(colors, reqcolors);

    /*
    ** Step 3: apply median-cut to histogram, making the new acolormap.
    */
    acolormap = mediancut(achv, colors, rows * cols, maxval, newcolors);
    pam_freeacolorhist(achv);


    /*
    ** Step 3.4 [GRR]: set the bit-depth appropriately, given the actual
    ** number of colors that will be used in the output image.
    */

    if (newcolors <= 2)
        rwpng_info->sample_depth = 1;
    else if (newcolors <= 4)
        rwpng_info->sample_depth = 2;
    else if (newcolors <= 16)
        rwpng_info->sample_depth = 4;
    else
        rwpng_info->sample_depth = 8;

    /*
    ** Step 3.5 [GRR]: remap the palette colors so that all entries with
    ** the maximal alpha value (i.e., fully opaque) are at the end and can
    ** therefore be omitted from the tRNS chunk.  Note that the ordering of
    ** opaque entries is reversed from how Step 3 arranged them--not that
    ** this should matter to anyone.
    */

    for (top_idx = newcolors-1, bot_idx = x = 0;  x < newcolors;  ++x) {
        if (PAM_GETA(acolormap[x].acolor) == maxval)
            remap[x] = top_idx--;
        else
            remap[x] = bot_idx++;
    }

    /* sanity check:  top and bottom indices should have just crossed paths */
    if (bot_idx != top_idx + 1) {
        CPE_ERROR(ctx->m_em,"internal logic error: remapped bot_idx = %d, top_idx = %d", bot_idx, top_idx);
        return -1;
    }

    rwpng_info->num_palette = newcolors;
    rwpng_info->num_trans = bot_idx;
    /* GRR TO DO:  if bot_idx == 0, check whether all RGB samples are gray
                   and if so, whether grayscale sample_depth would be same
                   => skip following palette section and go grayscale */


    /*
    ** Step 3.6 [GRR]: rescale the palette colors to a maxval of 255, as
    ** required by the PNG spec.  (Technically, the actual remapping happens
    ** in here, too.)
    */

    if (maxval < 255) {
        for (x = 0; x < newcolors; ++x) {
            /* the rescaling part of this is really just PAM_DEPTH() broken out
             *  for the PNG palette; the trans-remapping just puts the values
             *  in different slots in the PNG palette */
            rwpng_info->palette[remap[x]].red
              = (PAM_GETR(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
            rwpng_info->palette[remap[x]].green
              = (PAM_GETG(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
            rwpng_info->palette[remap[x]].blue
              = (PAM_GETB(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
            rwpng_info->trans[remap[x]]
              = (PAM_GETA(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
        }
        /* GRR TO DO:  set sBIT flag appropriately */
    }
    else {
        for (x = 0; x < newcolors; ++x) {
            rwpng_info->palette[remap[x]].red
              = PAM_GETR( acolormap[x].acolor );
            rwpng_info->palette[remap[x]].green
              = PAM_GETG( acolormap[x].acolor );
            rwpng_info->palette[remap[x]].blue
              = PAM_GETB( acolormap[x].acolor );
            rwpng_info->trans[remap[x]]
              = PAM_GETA( acolormap[x].acolor );
        }
    }


    /*
    ** Step 3.7 [GRR]: allocate memory for either a single row (non-
    ** interlaced -> progressive write) or the entire indexed image
    ** (interlaced -> all at once); note that rwpng_info->row_pointers
    ** is still in use via apixels (INPUT data).
    */

    if (rwpng_info->interlaced) {
        if ((rwpng_info->indexed_data = (uch *)malloc(rows * cols)) != NULL) {
            if ((row_pointers = (uch **)malloc(rows * sizeof(uch *))) != NULL) {
                for (row = 0;  (ulg)row < rows;  ++row)
                    row_pointers[row] = rwpng_info->indexed_data + row*cols;
            }
        }
    }
    else {
        rwpng_info->indexed_data = (uch *)malloc(cols);
    }

    if (rwpng_info->indexed_data == NULL || (rwpng_info->interlaced && row_pointers == NULL)) {
        CPE_ERROR(ctx->m_em, "insufficient memory for indexed data and/or row pointers");
        return -1;
    }



    /*
    ** Step 4: map the colors in the image to their closest match in the
    ** new colormap, and write 'em out.
    */
    acht = pam_allocacolorhash( );
    usehash = 1;

    assert(0);
    //TODO: Loki
    /* if (rwpng_write_image_init(output, rwpng_info) != 0) { */
    /*     CPE_ERROR(ctx->m_em, "  rwpng_write_image_init() error"); */
    /*     if (row_pointers) free(row_pointers); */
    /*     return -1; */
    /* } */

    if ( floyd ) {
        /* Initialize Floyd-Steinberg error vectors. */
        thisrerr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        nextrerr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        thisgerr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        nextgerr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        thisberr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        nextberr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        thisaerr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        nextaerr = (long*) pm_allocrow( cols + 2, sizeof(long) );
        srandom( (int) ( time( 0 ) ^ getpid( ) ) );
        for ( col = 0; (ulg)col < cols + 2; ++col ) {
            thisrerr[col] = random( ) % ( FS_SCALE * 2 ) - FS_SCALE;
            thisgerr[col] = random( ) % ( FS_SCALE * 2 ) - FS_SCALE;
            thisberr[col] = random( ) % ( FS_SCALE * 2 ) - FS_SCALE;
            thisaerr[col] = random( ) % ( FS_SCALE * 2 ) - FS_SCALE;
            /* (random errors in [-1 .. 1]) */
        }
        fs_direction = 1;
    }
    for ( row = 0; (ulg)row < rows; ++row ) {
        outrow = rwpng_info->interlaced? row_pointers[row] :
                                        rwpng_info->indexed_data;
        if ( floyd )
            for ( col = 0; (ulg)col < cols + 2; ++col )
                nextrerr[col] = nextgerr[col] =
                nextberr[col] = nextaerr[col] = 0;
        if ( ( ! floyd ) || fs_direction ) {
            col = 0;
            limitcol = cols;
            pP = apixels[row];
            pQ = outrow;
        } else {
            col = cols - 1;
            limitcol = -1;
            pP = &(apixels[row][col]);
            pQ = &(outrow[col]);
        }
        do {
            if ( floyd ) {
                /* Use Floyd-Steinberg errors to adjust actual color. */
                sr = PAM_GETR(*pP) + thisrerr[col + 1] / FS_SCALE;
                sg = PAM_GETG(*pP) + thisgerr[col + 1] / FS_SCALE;
                sb = PAM_GETB(*pP) + thisberr[col + 1] / FS_SCALE;
                sa = PAM_GETA(*pP) + thisaerr[col + 1] / FS_SCALE;
                if ( sr < 0 ) sr = 0;
                else if ( sr > maxval ) sr = maxval;
                if ( sg < 0 ) sg = 0;
                else if ( sg > maxval ) sg = maxval;
                if ( sb < 0 ) sb = 0;
                else if ( sb > maxval ) sb = maxval;
                if ( sa < 0 ) sa = 0;
                else if ( sa > maxval ) sa = maxval;
                /* GRR 20001228:  added casts to quiet warnings; 255 DEPENDENCY */
                PAM_ASSIGN( *pP, (uch)sr, (uch)sg, (uch)sb, (uch)sa );
            }

            /* Check hash table to see if we have already matched this color. */
            ind = pam_lookupacolor( acht, pP );
            if ( ind == -1 ) {
                /* No; search acolormap for closest match. */
                register int i, r1, g1, b1, a1, r2, g2, b2, a2;
                register long dist, newdist;

                r1 = PAM_GETR( *pP );
                g1 = PAM_GETG( *pP );
                b1 = PAM_GETB( *pP );
                a1 = PAM_GETA( *pP );
                dist = 2000000000;
                for ( i = 0; i < newcolors; ++i ) {
                    r2 = PAM_GETR( acolormap[i].acolor );
                    g2 = PAM_GETG( acolormap[i].acolor );
                    b2 = PAM_GETB( acolormap[i].acolor );
                    a2 = PAM_GETA( acolormap[i].acolor );
/* GRR POSSIBLE BUG */
                    newdist = ( r1 - r2 ) * ( r1 - r2 ) +  /* may overflow? */
                              ( g1 - g2 ) * ( g1 - g2 ) +
                              ( b1 - b2 ) * ( b1 - b2 ) +
                              ( a1 - a2 ) * ( a1 - a2 );
                    if ( newdist < dist ) {
                        ind = i;
                        dist = newdist;
                    }
                }
                if ( usehash ) {
                    if ( pam_addtoacolorhash( acht, pP, ind ) < 0 ) {
                        usehash = 0;
                    }
                }
            }

            if ( floyd ) {
                /* Propagate Floyd-Steinberg error terms. */
                if ( fs_direction ) {
                    err = (sr - (long)PAM_GETR(acolormap[ind].acolor))*FS_SCALE;
                    thisrerr[col + 2] += ( err * 7 ) / 16;
                    nextrerr[col    ] += ( err * 3 ) / 16;
                    nextrerr[col + 1] += ( err * 5 ) / 16;
                    nextrerr[col + 2] += ( err     ) / 16;
                    err = (sg - (long)PAM_GETG(acolormap[ind].acolor))*FS_SCALE;
                    thisgerr[col + 2] += ( err * 7 ) / 16;
                    nextgerr[col    ] += ( err * 3 ) / 16;
                    nextgerr[col + 1] += ( err * 5 ) / 16;
                    nextgerr[col + 2] += ( err     ) / 16;
                    err = (sb - (long)PAM_GETB(acolormap[ind].acolor))*FS_SCALE;
                    thisberr[col + 2] += ( err * 7 ) / 16;
                    nextberr[col    ] += ( err * 3 ) / 16;
                    nextberr[col + 1] += ( err * 5 ) / 16;
                    nextberr[col + 2] += ( err     ) / 16;
                    err = (sa - (long)PAM_GETA(acolormap[ind].acolor))*FS_SCALE;
                    thisaerr[col + 2] += ( err * 7 ) / 16;
                    nextaerr[col    ] += ( err * 3 ) / 16;
                    nextaerr[col + 1] += ( err * 5 ) / 16;
                    nextaerr[col + 2] += ( err     ) / 16;
                } else {
                    err = (sr - (long)PAM_GETR(acolormap[ind].acolor))*FS_SCALE;
                    thisrerr[col    ] += ( err * 7 ) / 16;
                    nextrerr[col + 2] += ( err * 3 ) / 16;
                    nextrerr[col + 1] += ( err * 5 ) / 16;
                    nextrerr[col    ] += ( err     ) / 16;
                    err = (sg - (long)PAM_GETG(acolormap[ind].acolor))*FS_SCALE;
                    thisgerr[col    ] += ( err * 7 ) / 16;
                    nextgerr[col + 2] += ( err * 3 ) / 16;
                    nextgerr[col + 1] += ( err * 5 ) / 16;
                    nextgerr[col    ] += ( err     ) / 16;
                    err = (sb - (long)PAM_GETB(acolormap[ind].acolor))*FS_SCALE;
                    thisberr[col    ] += ( err * 7 ) / 16;
                    nextberr[col + 2] += ( err * 3 ) / 16;
                    nextberr[col + 1] += ( err * 5 ) / 16;
                    nextberr[col    ] += ( err     ) / 16;
                    err = (sa - (long)PAM_GETA(acolormap[ind].acolor))*FS_SCALE;
                    thisaerr[col    ] += ( err * 7 ) / 16;
                    nextaerr[col + 2] += ( err * 3 ) / 16;
                    nextaerr[col + 1] += ( err * 5 ) / 16;
                    nextaerr[col    ] += ( err     ) / 16;
                }
            }

/*          *pP = acolormap[ind].acolor;  */
            *pQ = (uch)remap[ind];

            if ( ( ! floyd ) || fs_direction ) {
                ++col;
                ++pP;
                ++pQ;
            } else {
                --col;
                --pP;
                --pQ;
            }
        }
        while ( col != limitcol );

        if ( floyd ) {
            temperr = thisrerr;
            thisrerr = nextrerr;
            nextrerr = temperr;
            temperr = thisgerr;
            thisgerr = nextgerr;
            nextgerr = temperr;
            temperr = thisberr;
            thisberr = nextberr;
            nextberr = temperr;
            temperr = thisaerr;
            thisaerr = nextaerr;
            nextaerr = temperr;
            fs_direction = ! fs_direction;
        }

        /* if non-interlaced PNG, write row now */
        if (!rwpng_info->interlaced) {
            rwpng_write_image_row(rwpng_info);
        }
    }


    /* write entire interlaced palette PNG, or finish/flush noninterlaced one */
    if (rwpng_info->interlaced) {
        if (rwpng_info->row_pointers) free(rwpng_info->row_pointers);
        rwpng_info->row_pointers = row_pointers;   /* now for OUTPUT data */
        rwpng_write_image_whole(rwpng_info);
    }
    else {
        rwpng_write_image_finish(rwpng_info);
    }
    
    return 0;
}

/*
** Here is the fun part, the median-cut colormap generator.  This is based
** on Paul Heckbert's paper, "Color Image Quantization for Frame Buffer
** Display," SIGGRAPH 1982 Proceedings, page 297.
*/

static acolorhist_vector
mediancut(acolorhist_vector achv, int colors, int sum, pixval maxval, int newcolors) {
    acolorhist_vector acolormap;
    box_vector bv;
    register int bi, i;
    int boxes;

    bv = (box_vector) malloc( sizeof(struct box) * newcolors );
    acolormap =
        (acolorhist_vector) malloc( sizeof(struct acolorhist_item) * newcolors);
    if ( bv == (box_vector) 0 || acolormap == (acolorhist_vector) 0 ) {
        fprintf( stderr, "  out of memory allocating box vector\n" );
        fflush(stderr);
        exit(6);
    }
    for ( i = 0; i < newcolors; ++i )
        PAM_ASSIGN( acolormap[i].acolor, 0, 0, 0, 0 );

    /*
    ** Set up the initial box.
    */
    bv[0].ind = 0;
    bv[0].colors = colors;
    bv[0].sum = sum;
    boxes = 1;

    /*
    ** Main loop: split boxes until we have enough.
    */
    while ( boxes < newcolors ) {
        register int indx, clrs;
        int sm;
        register int minr, maxr, ming, mina, maxg, minb, maxb, maxa, v;
        int halfsum, lowersum;

        /*
        ** Find the first splittable box.
        */
        for ( bi = 0; bi < boxes; ++bi )
            if ( bv[bi].colors >= 2 )
                break;
        if ( bi == boxes )
            break;        /* ran out of colors! */
        indx = bv[bi].ind;
        clrs = bv[bi].colors;
        sm = bv[bi].sum;

        /*
        ** Go through the box finding the minimum and maximum of each
        ** component - the boundaries of the box.
        */
        minr = maxr = PAM_GETR( achv[indx].acolor );
        ming = maxg = PAM_GETG( achv[indx].acolor );
        minb = maxb = PAM_GETB( achv[indx].acolor );
        mina = maxa = PAM_GETA( achv[indx].acolor );
        for ( i = 1; i < clrs; ++i )
            {
            v = PAM_GETR( achv[indx + i].acolor );
            if ( v < minr ) minr = v;
            if ( v > maxr ) maxr = v;
            v = PAM_GETG( achv[indx + i].acolor );
            if ( v < ming ) ming = v;
            if ( v > maxg ) maxg = v;
            v = PAM_GETB( achv[indx + i].acolor );
            if ( v < minb ) minb = v;
            if ( v > maxb ) maxb = v;
            v = PAM_GETA( achv[indx + i].acolor );
            if ( v < mina ) mina = v;
            if ( v > maxa ) maxa = v;
            }

        /*
        ** Find the largest dimension, and sort by that component.  I have
        ** included two methods for determining the "largest" dimension;
        ** first by simply comparing the range in RGB space, and second
        ** by transforming into luminosities before the comparison.  You
        ** can switch which method is used by switching the commenting on
        ** the LARGE_ defines at the beginning of this source file.
        */
#ifdef LARGE_NORM
        if ( maxa - mina >= maxr - minr && maxa - mina >= maxg - ming && maxa - mina >= maxb - minb )
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                alphacompare );
        else if ( maxr - minr >= maxg - ming && maxr - minr >= maxb - minb )
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                redcompare );
        else if ( maxg - ming >= maxb - minb )
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                greencompare );
        else
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                bluecompare );
#endif /*LARGE_NORM*/
#ifdef LARGE_LUM
        {
        apixel p;
        float rl, gl, bl, al;

        PAM_ASSIGN(p, maxr - minr, 0, 0, 0);
        rl = PPM_LUMIN(p);
        PAM_ASSIGN(p, 0, maxg - ming, 0, 0);
        gl = PPM_LUMIN(p);
        PAM_ASSIGN(p, 0, 0, maxb - minb, 0);
        bl = PPM_LUMIN(p);

/*
GRR: treat alpha as grayscale and assign (maxa - mina) to each of R, G, B?
     assign (maxa - mina)/3 to each?
     use alpha-fractional luminosity?  (normalized_alpha * lum(r,g,b))
        al = dunno ...
     [probably should read Heckbert's paper to decide]
 */

        if ( al >= rl && al >= gl && al >= bl )
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                alphacompare );
        else if ( rl >= gl && rl >= bl )
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                redcompare );
        else if ( gl >= bl )
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                greencompare );
        else
            qsort(
                (char*) &(achv[indx]), clrs, sizeof(struct acolorhist_item),
                bluecompare );
        }
#endif /*LARGE_LUM*/

        /*
        ** Now find the median based on the counts, so that about half the
        ** pixels (not colors, pixels) are in each subdivision.
        */
        lowersum = achv[indx].value;
        halfsum = sm / 2;
        for ( i = 1; i < clrs - 1; ++i )
            {
            if ( lowersum >= halfsum )
                break;
            lowersum += achv[indx + i].value;
            }

        /*
        ** Split the box, and sort to bring the biggest boxes to the top.
        */
        bv[bi].colors = i;
        bv[bi].sum = lowersum;
        bv[boxes].ind = indx + i;
        bv[boxes].colors = clrs - i;
        bv[boxes].sum = sm - lowersum;
        ++boxes;
        qsort( (char*) bv, boxes, sizeof(struct box), sumcompare );
    }

    /*
    ** Ok, we've got enough boxes.  Now choose a representative color for
    ** each box.  There are a number of possible ways to make this choice.
    ** One would be to choose the center of the box; this ignores any structure
    ** within the boxes.  Another method would be to average all the colors in
    ** the box - this is the method specified in Heckbert's paper.  A third
    ** method is to average all the pixels in the box.  You can switch which
    ** method is used by switching the commenting on the REP_ defines at
    ** the beginning of this source file.
    */
    for ( bi = 0; bi < boxes; ++bi ) {
#ifdef REP_CENTER_BOX
        register int indx = bv[bi].ind;
        register int clrs = bv[bi].colors;
        register int minr, maxr, ming, maxg, minb, maxb, mina, maxa, v;

        minr = maxr = PAM_GETR( achv[indx].acolor );
        ming = maxg = PAM_GETG( achv[indx].acolor );
        minb = maxb = PAM_GETB( achv[indx].acolor );
        mina = maxa = PAM_GETA( achv[indx].acolor );
        for ( i = 1; i < clrs; ++i )
            {
            v = PAM_GETR( achv[indx + i].acolor );
            minr = min( minr, v );
            maxr = max( maxr, v );
            v = PAM_GETG( achv[indx + i].acolor );
            ming = min( ming, v );
            maxg = max( maxg, v );
            v = PAM_GETB( achv[indx + i].acolor );
            minb = min( minb, v );
            maxb = max( maxb, v );
            v = PAM_GETA( achv[indx + i].acolor );
            mina = min( mina, v );
            maxa = max( maxa, v );
            }
        PAM_ASSIGN(
            acolormap[bi].acolor, ( minr + maxr ) / 2, ( ming + maxg ) / 2,
            ( minb + maxb ) / 2, ( mina + maxa ) / 2 );
#endif /*REP_CENTER_BOX*/
#ifdef REP_AVERAGE_COLORS
        register int indx = bv[bi].ind;
        register int clrs = bv[bi].colors;
        register long r = 0, g = 0, b = 0, a = 0;

        for ( i = 0; i < clrs; ++i )
            {
            r += PAM_GETR( achv[indx + i].acolor );
            g += PAM_GETG( achv[indx + i].acolor );
            b += PAM_GETB( achv[indx + i].acolor );
            a += PAM_GETA( achv[indx + i].acolor );
            }
        r = r / clrs;
        g = g / clrs;
        b = b / clrs;
        a = a / clrs;
        PAM_ASSIGN( acolormap[bi].acolor, r, g, b, a );
#endif /*REP_AVERAGE_COLORS*/
#ifdef REP_AVERAGE_PIXELS
        register int indx = bv[bi].ind;
        register int clrs = bv[bi].colors;
        register long r = 0, g = 0, b = 0, a = 0, sum = 0;

        for ( i = 0; i < clrs; ++i )
            {
            r += PAM_GETR( achv[indx + i].acolor ) * achv[indx + i].value;
            g += PAM_GETG( achv[indx + i].acolor ) * achv[indx + i].value;
            b += PAM_GETB( achv[indx + i].acolor ) * achv[indx + i].value;
            a += PAM_GETA( achv[indx + i].acolor ) * achv[indx + i].value;
            sum += achv[indx + i].value;
            }
        r = r / sum;
        if ( r > maxval ) r = maxval;        /* avoid math errors */
        g = g / sum;
        if ( g > maxval ) g = maxval;
        b = b / sum;
        if ( b > maxval ) b = maxval;
        a = a / sum;
        if ( a > maxval ) a = maxval;
        /* GRR 20001228:  added casts to quiet warnings; 255 DEPENDENCY */
        PAM_ASSIGN( acolormap[bi].acolor, (uch)r, (uch)g, (uch)b, (uch)a );
#endif /*REP_AVERAGE_PIXELS*/
    }

    /*
    ** All done.
    */
    return acolormap;
}

static int redcompare( const void *ch1, const void *ch2 ) {
    return (int) PAM_GETR( ((acolorhist_vector)ch1)->acolor ) -
           (int) PAM_GETR( ((acolorhist_vector)ch2)->acolor );
}

static int greencompare( const void *ch1, const void *ch2 ) {
    return (int) PAM_GETG( ((acolorhist_vector)ch1)->acolor ) -
           (int) PAM_GETG( ((acolorhist_vector)ch2)->acolor );
}

static int bluecompare( const void *ch1, const void *ch2 ) {
    return (int) PAM_GETB( ((acolorhist_vector)ch1)->acolor ) -
           (int) PAM_GETB( ((acolorhist_vector)ch2)->acolor );
}

static int alphacompare( const void *ch1, const void *ch2) {
    return (int) PAM_GETA( ((acolorhist_vector)ch1)->acolor ) -
           (int) PAM_GETA( ((acolorhist_vector)ch2)->acolor );
}

static int sumcompare( const void *b1, const void *b2 ) {
    return ((box_vector)b2)->sum -
           ((box_vector)b1)->sum;
}



/*

libpam3.c:
	pam_computeacolorhist( )
NOTUSED	pam_addtoacolorhist( )
	pam_computeacolorhash( )
	pam_allocacolorhash( )
	pam_addtoacolorhash( )
	pam_acolorhashtoacolorhist( )
NOTUSED	pam_acolorhisttoacolorhash( )
	pam_lookupacolor( )
	pam_freeacolorhist( )
	pam_freeacolorhash( )

libpbm1.c:
	pm_allocrow( )

pam.h:
	pam_freearray( )
 */


/*===========================================================================*/


/* libpam3.c - pam (portable alpha map) utility library part 3
**
** Colormap routines.
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
** Copyright (C) 1997 by Greg Roelofs.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/*
#include "pam.h"
#include "pamcmap.h"
 */

#define HASH_SIZE 20023

#define pam_hashapixel(p) ( ( ( (long) PAM_GETR(p) * 33023 + \
                                (long) PAM_GETG(p) * 30013 + \
                                (long) PAM_GETB(p) * 27011 + \
                                (long) PAM_GETA(p) * 24007 ) \
                              & 0x7fffffff ) % HASH_SIZE )

static acolorhist_vector
pam_computeacolorhist(apixel** apixels, int cols, int rows, int maxacolors, int * acolorsP) {
    acolorhash_table acht;
    acolorhist_vector achv;

    acht = pam_computeacolorhash( apixels, cols, rows, maxacolors, acolorsP );
    if (acht == NULL) return NULL;

    achv = pam_acolorhashtoacolorhist(acht, maxacolors);
    pam_freeacolorhash( acht );
    
    return achv;
}

static acolorhash_table
pam_computeacolorhash(apixel** apixels, int cols, int rows, int maxacolors, int * acolorsP) {
    acolorhash_table acht;
    register apixel* pP;
    acolorhist_list achl;
    int col, row, hash;

    acht = pam_allocacolorhash( );
    *acolorsP = 0;

    /* Go through the entire image, building a hash table of colors. */
    for (row = 0; row < rows; ++row) {
        for (col = 0, pP = apixels[row]; col < cols; ++col, ++pP) {
            hash = pam_hashapixel( *pP );
            for (achl = acht[hash]; achl; achl = achl->next) {
                if (PAM_EQUAL(achl->ch.acolor, *pP)) break;
            }
            
            if (achl) {
                ++(achl->ch.value);
            }
            else {
                if ( ++(*acolorsP) > maxacolors ) {
                    pam_freeacolorhash(acht);
                    return NULL;
                }
                
                achl = (acolorhist_list) malloc( sizeof(struct acolorhist_list_item) );
                if (achl == NULL) {
                    fprintf( stderr, "  out of memory computing hash table\n" );
                    exit(7);
                }
                achl->ch.acolor = *pP;
                achl->ch.value = 1;
                achl->next = acht[hash];
                acht[hash] = achl;
            }
	    }
    }
    
    return acht;
}

static acolorhash_table pam_allocacolorhash() {
    acolorhash_table acht;
    int i;

    acht = (acolorhash_table) malloc( HASH_SIZE * sizeof(acolorhist_list) );
    if (acht == NULL) {
        fprintf( stderr, "  out of memory allocating hash table\n" );
        exit(8);
    }

    for (i = 0; i < HASH_SIZE; ++i) {
        acht[i] = NULL;
    }

    return acht;
}

static int pam_addtoacolorhash(acolorhash_table acht, apixel* acolorP, int value) {
    register int hash;
    register acolorhist_list achl;

    achl = (acolorhist_list) malloc( sizeof(struct acolorhist_list_item) );
    if (achl == NULL) return -1;
    
    hash = pam_hashapixel( *acolorP );
    achl->ch.acolor = *acolorP;
    achl->ch.value = value;
    achl->next = acht[hash];
    acht[hash] = achl;
    return 0;
}

static acolorhist_vector pam_acolorhashtoacolorhist(acolorhash_table acht, int maxacolors) {
    acolorhist_vector achv;
    acolorhist_list achl;
    int i, j;

    /* Now collate the hash table into a simple acolorhist array. */
    achv = (acolorhist_vector) malloc( maxacolors * sizeof(struct acolorhist_item) );
    /* (Leave room for expansion by caller.) */
    if (achv == NULL) {
        fprintf( stderr, "  out of memory generating histogram\n");
        exit(9);
    }

    /* Loop through the hash table. */
    j = 0;
    for ( i = 0; i < HASH_SIZE; ++i ) {
        for ( achl = acht[i]; achl != (acolorhist_list) 0; achl = achl->next ) {
            /* Add the new entry. */
            achv[j] = achl->ch;
            ++j;
	    }
    }

    /* All done. */
    return achv;
}

static int pam_lookupacolor(acolorhash_table acht, apixel* acolorP) {
    int hash;
    acolorhist_list achl;

    hash = pam_hashapixel( *acolorP );
    for (achl = acht[hash]; achl != (acolorhist_list) 0; achl = achl->next) {
        if (PAM_EQUAL( achl->ch.acolor, *acolorP)) return achl->ch.value;
    }

    return -1;
}

static void pam_freeacolorhist(acolorhist_vector achv) {
    free((char*)achv);
}

static void pam_freeacolorhash(acolorhash_table acht) {
    int i;
    acolorhist_list achl, achlnext;

    for (i = 0; i < HASH_SIZE; ++i) {
        for (achl = acht[i]; achl != (acolorhist_list) 0; achl = achlnext) {
            achlnext = achl->next;
            free( (char*) achl );
	    }
    }
    
    free((char*) acht);
}

static char*
pm_allocrow(int cols, int size) {
    register char* itrow;

    itrow = (char*) malloc( cols * size );
    if ( itrow == (char*) 0 ) {
        fprintf( stderr, "  out of memory allocating a row\n" );
        fflush( stderr );
        exit(12);
    }
    
    return itrow;
}

static void convert_save_res_clear_image_info(convert_ctx_t ctx, mainprog_info *rwpng_info) {
    if (rwpng_info->rgba_data) {
        free(rwpng_info->rgba_data);
        rwpng_info->rgba_data = NULL;
    }
    
    if (rwpng_info->row_pointers) {
        free(rwpng_info->row_pointers);
        rwpng_info->row_pointers = NULL;
    }
    
    if (rwpng_info->indexed_data) {
        free(rwpng_info->indexed_data);
        rwpng_info->indexed_data = NULL;
    }
}
