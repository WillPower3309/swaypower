#ifndef _SWAY_OPENGL_H
#define _SWAY_OPENGL_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdbool.h>
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/addon.h>
#include <wlr/util/box.h>
#include "sway/desktop/fx_renderer/fx_stencilbuffer.h"

enum corner_location { TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, ALL, NONE };

enum fx_tex_shader_source {
	SHADER_SOURCE_TEXTURE_RGBA = 1,
	SHADER_SOURCE_TEXTURE_RGBX = 2,
	SHADER_SOURCE_TEXTURE_EXTERNAL = 3,
};

enum fx_rounded_quad_shader_source {
	SHADER_SOURCE_QUAD_ROUND = 1,
	SHADER_SOURCE_QUAD_ROUND_TOP_LEFT = 2,
	SHADER_SOURCE_QUAD_ROUND_TOP_RIGHT = 3,
	SHADER_SOURCE_QUAD_ROUND_BOTTOM_RIGHT = 4,
	SHADER_SOURCE_QUAD_ROUND_BOTTOM_LEFT = 5,
};

struct decoration_data {
	float alpha;
	float saturation;
	int corner_radius;
	float dim;
	float *dim_color;
	bool has_titlebar;
	bool discard_transparent;
	bool blur;
	bool shadow;
};

struct blur_shader {
	GLuint program;
	GLint proj;
	GLint tex;
	GLint pos_attrib;
	GLint tex_attrib;
	GLint radius;
	GLint halfpixel;
};

struct box_shadow_shader {
	GLuint program;
	GLint proj;
	GLint color;
	GLint pos_attrib;
	GLint position;
	GLint size;
	GLint blur_sigma;
	GLint corner_radius;
};

struct corner_shader {
	GLuint program;
	GLint proj;
	GLint color;
	GLint pos_attrib;
	GLint is_top_left;
	GLint is_top_right;
	GLint is_bottom_left;
	GLint is_bottom_right;
	GLint position;
	GLint radius;
	GLint half_size;
	GLint half_thickness;
};

struct quad_shader {
	GLuint program;
	GLint proj;
	GLint color;
	GLint pos_attrib;
};

struct rounded_quad_shader {
	GLuint program;
	GLint proj;
	GLint color;
	GLint pos_attrib;
	GLint size;
	GLint position;
	GLint radius;
};

struct stencil_mask_shader {
	GLuint program;
	GLint proj;
	GLint color;
	GLint pos_attrib;
	GLint half_size;
	GLint position;
	GLint radius;
};

struct tex_shader {
	GLuint program;
	GLint proj;
	GLint tex;
	GLint alpha;
	GLint pos_attrib;
	GLint tex_attrib;
	GLint size;
	GLint position;
	GLint radius;
	GLint saturation;
	GLint dim;
	GLint dim_color;
	GLint has_titlebar;
	GLint discard_transparent;
};

struct fx_framebuffer {
	bool initialized;

	GLuint fbo;
	GLuint rbo;

	struct wlr_buffer *wlr_buffer;
	struct fx_renderer *renderer;
	struct wl_list link; // fx_renderer.buffers
	struct wlr_addon addon;

	EGLImageKHR image;
};

struct fx_texture {
	struct wlr_texture wlr_texture;
	struct fx_renderer *fx_renderer;
	struct wl_list link; // fx_renderer.textures

	// Basically:
	//   GL_TEXTURE_2D == mutable
	//   GL_TEXTURE_EXTERNAL_OES == immutable
	GLuint target;
	GLuint tex;

	EGLImageKHR image;

	bool has_alpha;

	// Only affects target == GL_TEXTURE_2D
	uint32_t drm_format; // used to interpret upload data
	// If imported from a wlr_buffer
	struct wlr_buffer *buffer;
	struct wlr_addon buffer_addon;
};

struct fx_renderer {
	float projection[9];

	int viewport_width, viewport_height;

	struct wlr_output *wlr_output;

	struct wlr_egl *egl;

	struct fx_stencilbuffer stencil_buffer;

	struct wl_list textures; // fx_texture.link
	struct wl_list buffers; // fx_framebuffer.link

	// The FBO and texture used by wlroots
	GLuint wlr_main_buffer_fbo;
	struct wlr_gles2_texture_attribs wlr_main_texture_attribs;

	const struct wlr_drm_format *drm_format;

	// Contains the blurred background for tiled windows
	struct fx_framebuffer blur_buffer;
	// Contains the original pixels to draw over the areas where artifact are visible
	struct fx_framebuffer blur_saved_pixels_buffer;
	// Blur swaps between the two effects buffers everytime it scales the image
	// Buffer used for effects
	struct fx_framebuffer effects_buffer;
	// Swap buffer used for effects
	struct fx_framebuffer effects_buffer_swapped;

	// The region where there's blur
	pixman_region32_t blur_padding_region;

	bool blur_buffer_dirty;

	struct {
		bool OES_egl_image_external;
		bool OES_egl_image;
	} exts;

	struct {
		PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
		PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glEGLImageTargetRenderbufferStorageOES;
	} procs;

	struct {
		struct box_shadow_shader box_shadow;
		struct blur_shader blur1;
		struct blur_shader blur2;
		struct corner_shader corner;
		struct quad_shader quad;
		struct rounded_quad_shader rounded_quad;
		struct rounded_quad_shader rounded_tl_quad;
		struct rounded_quad_shader rounded_tr_quad;
		struct rounded_quad_shader rounded_bl_quad;
		struct rounded_quad_shader rounded_br_quad;
		struct stencil_mask_shader stencil_mask;
		struct tex_shader tex_rgba;
		struct tex_shader tex_rgbx;
		struct tex_shader tex_ext;
	} shaders;
};

///
/// fx_framebuffer
///

struct fx_framebuffer fx_framebuffer_create(void);

void fx_framebuffer_bind(struct fx_framebuffer *buffer);

void fx_framebuffer_bind_wlr_fbo(struct fx_renderer *renderer);

void fx_framebuffer_update(struct fx_renderer *fx_renderer, struct fx_framebuffer *fx_buffer,
		int width, int height);

void fx_framebuffer_add_stencil_buffer(struct fx_framebuffer *buffer, int width, int height);

void fx_framebuffer_release(struct fx_framebuffer *buffer);

///
/// fx_texture
///

struct fx_texture *fx_get_texture(struct wlr_texture *wlr_texture);

struct fx_texture *fx_texture_from_buffer(struct fx_renderer *fx_renderer,
	struct wlr_buffer *buffer);

void fx_texture_destroy(struct fx_texture *texture);

bool wlr_texture_is_fx(struct wlr_texture *wlr_texture);

void wlr_gles2_texture_get_fx_attribs(struct fx_texture *texture,
		struct wlr_gles2_texture_attribs *attribs);

///
/// fx_renderer
///

struct fx_renderer *fx_renderer_create(struct wlr_egl *egl, struct wlr_output *output);

void fx_renderer_fini(struct fx_renderer *renderer);

void fx_renderer_begin(struct fx_renderer *renderer, int width, int height);

void fx_renderer_end(struct fx_renderer *renderer);

void fx_renderer_clear(const float color[static 4]);

void fx_renderer_scissor(struct wlr_box *box);

void fx_renderer_get_texture_attribs(struct wlr_texture *texture,
		struct wlr_gles2_texture_attribs *attribs);

// Initialize the stenciling work
void fx_renderer_stencil_mask_init();

// Close the mask
void fx_renderer_stencil_mask_close(bool draw_inside_mask);

// Finish stenciling and clear the buffer
void fx_renderer_stencil_mask_fini();

bool fx_render_subtexture_with_matrix(struct fx_renderer *renderer, struct wlr_texture *wlr_texture,
		const struct wlr_fbox *src_box, const struct wlr_box *dst_box, const float matrix[static 9],
		struct decoration_data deco_data);

bool fx_render_texture_with_matrix(struct fx_renderer *renderer, struct wlr_texture *wlr_texture,
		const struct wlr_box *dst_box, const float matrix[static 9], struct decoration_data deco_data);

void fx_render_rect(struct fx_renderer *renderer, const struct wlr_box *box,
		const float color[static 4], const float projection[static 9]);

void fx_render_rounded_rect(struct fx_renderer *renderer, const struct wlr_box *box,
		const float color[static 4], const float matrix[static 9], int radius,
		enum corner_location corner_location);

void fx_render_border_corner(struct fx_renderer *renderer, const struct wlr_box *box,
		const float color[static 4], const float matrix[static 9],
		enum corner_location corner_location, int radius, int border_thickness);

void fx_render_box_shadow(struct fx_renderer *renderer, const struct wlr_box *box,
		const float color[static 4], const float matrix[static 9], int radius,
		float blur_sigma);

void fx_render_blur(struct fx_renderer *renderer, const float matrix[static 9],
		struct wlr_gles2_texture_attribs *texture, struct blur_shader *shader,
		const struct wlr_box *box, int blur_radius);

#endif
