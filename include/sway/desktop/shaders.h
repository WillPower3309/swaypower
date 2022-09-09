#ifndef _SWAY_SHADERS_H
#define _SWAY_SHADERS_H

// Colored quads
const GLchar quad_vertex_src[] =
"uniform mat3 proj;\n"
"uniform vec4 color;\n"
"attribute vec2 pos;\n"
"attribute vec2 texcoord;\n"
"varying vec4 v_color;\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"	gl_Position = vec4(proj * vec3(pos, 1.0), 1.0);\n"
"	v_color = color;\n"
"	v_texcoord = texcoord;\n"
"}\n";

const GLchar quad_fragment_src[] =
"precision mediump float;\n"
"varying vec4 v_color;\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"	gl_FragColor = v_color;\n"
"}\n";

// Textured quads
const GLchar tex_vertex_src[] =
"uniform mat3 proj;\n"
"attribute vec2 pos;\n"
"attribute vec2 texcoord;\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"	gl_Position = vec4(proj * vec3(pos, 1.0), 1.0);\n"
"	v_texcoord = texcoord;\n"
"}\n";

// https://www.shadertoy.com/view/ltS3zW
const GLchar tex_fragment_src_rgba[] =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"\n"
"uniform vec2 half_size;\n"
"uniform vec2 position;\n"
"uniform float alpha;\n"
"uniform float radius;\n"
"uniform float half_border_thickness;\n"
"uniform vec4 border_color;\n"
"\n"
"float RectSDF(vec2 p, vec2 b, float r) {\n"
"   vec2 d = abs(p) - b + vec2(r);\n"
"	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;\n"
"}\n"
"\n"
"void main() {\n"
"	vec2 center = gl_FragCoord.xy - position - half_size;\n"
"	float dist = RectSDF(center, half_size + half_border_thickness, radius - half_border_thickness);\n"
"\n"
"	vec4 tex_color  = texture2D(tex, v_texcoord) * alpha;\n"
"   vec4 from_color = border_color;\n"
"   vec4 to_color   = vec4(0.0);\n"
"	if (half_border_thickness > 0.0) {\n"
"		if (dist < 0.0) {\n"
"			to_color = tex_color;\n"
"		}\n"
"		dist = abs(dist) - half_border_thickness;\n"
"	} else {\n"
"		from_color = tex_color;\n"
"	}\n"
"\n"
"	float blend_amount = smoothstep(-1.0, 1.0, dist);\n"
"	gl_FragColor = mix(from_color, to_color, blend_amount);\n"
"}\n";

const GLchar tex_fragment_src_rgbx[] =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"uniform float alpha;\n"
"\n"
"uniform vec2 size;\n"
"uniform vec2 position;\n"
"uniform float radius;\n"
"uniform float half_border_thickness;\n"
"\n"
"void main() {\n"
"	gl_FragColor = vec4(texture2D(tex, v_texcoord).rgb, 1.0) * alpha;\n"
"}\n";

const GLchar tex_fragment_src_external[] =
"#extension GL_OES_EGL_image_external : require\n\n"
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform samplerExternalOES texture0;\n"
"uniform float alpha;\n"
"\n"
"uniform vec2 size;\n"
"uniform vec2 position;\n"
"uniform float radius;\n"
"uniform float half_border_thickness;\n"
"\n"
"void main() {\n"
"	gl_FragColor = texture2D(texture0, v_texcoord) * alpha;\n"
"}\n";

#endif
