#include "render/cache/ui_cache_texture.h"
#include "plugin_swf_video_i.hpp"

plugin_swf_video_handler::plugin_swf_video_handler(plugin_swf_module_t module)
    : m_module(module)
    , m_texture(NULL)
    , m_scoord(0)
    , m_tcoord(0)
    , m_background_color(0,0,0,0)	// current background color
{
}

plugin_swf_video_handler::~plugin_swf_video_handler() {
    if (m_texture) {
        ui_cache_res_free(m_texture);
        m_texture = NULL;
    }
}

void plugin_swf_video_handler::display(
    Uint8* data, int width, int height, 
    const gameswf::matrix* m, const gameswf::rect* bounds, const gameswf::rgba& color)
{
//     // this can't be placed in constructor becuase opengl may not be accessible yet
//     if (m_texture == 0)
//     {
//         glEnable(GL_TEXTURE_2D);
//         glGenTextures(1, &m_texture);
//         glBindTexture(GL_TEXTURE_2D, m_texture);

//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// GL_NEAREST ?
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     }

//     glBindTexture(GL_TEXTURE_2D, m_texture);
//     glEnable(GL_TEXTURE_2D);

// 	//	glDisable(GL_TEXTURE_GEN_S);
// 	//	glDisable(GL_TEXTURE_GEN_T);

//     // update texture from video frame
//     if (data)
//     {
//         int w2p = p2(width);
//         int h2p = p2(height);
//         m_scoord = (float) width / w2p;
//         m_tcoord = (float) height / h2p;

//         if (m_clear_background)
//         {
//             // set video background color
//             // assume left-top pixel of the first frame as background color
//             if (m_background_color.m_a == 0)
//             {
//                 m_background_color.m_a = 255;
//                 m_background_color.m_r = data[2];
//                 m_background_color.m_g = data[1];
//                 m_background_color.m_b = data[0];
//             }

//             // clear video background, input data has BGRA format
//             Uint8* p = data;
//             for (int y = 0; y < height; y++)
//             {
//                 for (int x = 0; x < width; x++)
//                 {
//                     // calculate color distance, dist is in [0..195075]
//                     int r = m_background_color.m_r - p[2];
//                     int g = m_background_color.m_g - p[1];
//                     int b = m_background_color.m_b - p[0];
//                     float dist = (float) (r * r + g * g + b * b);

//                     static int s_min_dist = 3 * 64 * 64;	// hack
//                     Uint8 a = (dist < s_min_dist) ? (Uint8) (255 * (dist / s_min_dist)) : 255;

//                     p[3] = a;		// set alpha
//                     p += 4;
//                 }
//             }
//         }

//         // don't use compressed texture for video, it slows down video
//         //			ogl::create_texture(GL_RGBA, m_width2p, m_height2p, NULL);
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2p, h2p, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);
//     }

//     if (m_scoord == 0.0f && m_scoord == 0.0f)
//     {
//         // no data
//         return;
//     }

//     gameswf::point a, b, c, d;
//     m->transform(&a, gameswf::point(bounds->m_x_min, bounds->m_y_min));
//     m->transform(&b, gameswf::point(bounds->m_x_max, bounds->m_y_min));
//     m->transform(&c, gameswf::point(bounds->m_x_min, bounds->m_y_max));
//     d.m_x = b.m_x + c.m_x - a.m_x;
//     d.m_y = b.m_y + c.m_y - a.m_y;

//     glColor4ub(color.m_r, color.m_g, color.m_b, color.m_a);

// //		glBegin(GL_TRIANGLE_STRIP);
// //		{
// //			glTexCoord2f(0, 0);
// //			glVertex2f(a.m_x, a.m_y);
// //			glTexCoord2f(m_scoord, 0);
// //			glVertex2f(b.m_x, b.m_y);
// //			glTexCoord2f(0, m_tcoord);
// //			glVertex2f(c.m_x, c.m_y);
// //			glTexCoord2f(m_scoord, m_tcoord);
// //			glVertex2f(d.m_x, d.m_y);
// //		}
// //		glEnd();

//     // this code is equal to code that above

//     GLfloat squareVertices[8]; 
//     squareVertices[0] = a.m_x;
//     squareVertices[1] = a.m_y;
//     squareVertices[2] = b.m_x;
//     squareVertices[3] = b.m_y;
//     squareVertices[4] = c.m_x;
//     squareVertices[5] = c.m_y;
//     squareVertices[6] = d.m_x;
//     squareVertices[7] = d.m_y;

//     GLfloat squareTextureCoords[8];
//     squareTextureCoords[0] = 0;
//     squareTextureCoords[1] = 0;
//     squareTextureCoords[2] = m_scoord;
//     squareTextureCoords[3] = 0;
//     squareTextureCoords[4] = 0;
//     squareTextureCoords[5] = m_tcoord;
//     squareTextureCoords[6] = m_scoord;
//     squareTextureCoords[7] = m_tcoord;

//     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//     glTexCoordPointer(2, GL_FLOAT, 0, squareTextureCoords);

//     glEnableClientState(GL_VERTEX_ARRAY);
//     glVertexPointer(2, GL_FLOAT, 0, squareVertices);

//     glEnable(GL_LINE_SMOOTH);
//     glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
//     glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//     glDisable(GL_LINE_SMOOTH);

//     glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//     glDisableClientState(GL_VERTEX_ARRAY);

//     glDisable(GL_TEXTURE_2D);
}



