#if defined(IN_EDITOR) || defined(_WIN32)
const char* DefaultVS = " \
attribute	vec4	POSITION;	\
attribute	vec2	TEXCOORD0;	\
attribute	vec4	COLOR;		\
\
varying		vec2	texcoord0;	\
varying		vec4	color;		\
\
uniform		mat4	matrixMVP;	\
\
void main() \
{ \
    gl_Position = matrixMVP * POSITION; \
\
    texcoord0 = TEXCOORD0; \
\
	color	= COLOR / 255.0; \
}";
#else
const char* DefaultVS = " \
attribute	vec4	POSITION;	\
attribute	vec2	TEXCOORD0;	\
attribute	vec4	COLOR;		\
\
varying		highp vec2	texcoord0;	\
varying		lowp vec4	color;		\
\
uniform		mat4	matrixMVP;	\
\
void main() \
{ \
    gl_Position = matrixMVP * POSITION; \
\
    texcoord0 = TEXCOORD0; \
\
	color	= COLOR / 255.0; \
}";
#endif

#if defined(IN_EDITOR) || defined(_WIN32)
const char* DefaultPS_Modulate = "  \
varying		vec2		texcoord0;	\
varying		vec4		color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
    gl_FragColor = texture2D(texture0, texcoord0) * color; \
}";

const char* DefaultPS_Add = " \
varying		vec2		texcoord0;	\
varying		vec4		color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
	vec4 texColor = texture2D(texture0, texcoord0); \
	vec4 temp; \
	temp.a = texColor.a * color.a; \
	temp.r = texColor.r + color.r; \
	temp.g = texColor.g + color.g; \
	temp.b = texColor.b + color.b; \
    gl_FragColor = temp; \
}";

const char* DefaultPS_Color = " \
varying		vec2		texcoord0;	\
varying		vec4		color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
	vec4 texColor = texture2D(texture0, texcoord0); \
	vec4 temp; \
	temp.a = texColor.a * color.a; \
	temp.r = color.r; \
	temp.g = color.g; \
	temp.b = color.b; \
    gl_FragColor = temp; \
}";

const char* DefaultPS_Tex = " \
varying		vec2		texcoord0;	\
varying		vec4		color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
    gl_FragColor = texture2D(texture0, texcoord0); \
}";

const char* DefaultPS_Vtx = " \
varying		vec2		texcoord0;	\
varying		vec4		color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
    gl_FragColor = color; \
}";

#else
const char* DefaultPS_Modulate = "  \
varying		highp vec2	texcoord0;	\
varying		lowp vec4	color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
    gl_FragColor = texture2D(texture0, texcoord0) * color; \
}";

const char* DefaultPS_Add = " \
varying		highp vec2	texcoord0;	\
varying		lowp vec4	color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
	lowp vec4 texColor = texture2D(texture0, texcoord0); \
	lowp vec4 temp; \
	temp.a = texColor.a * color.a; \
	temp.r = texColor.r + color.r; \
	temp.g = texColor.g + color.g; \
	temp.b = texColor.b + color.b; \
    gl_FragColor = temp; \
}";

const char* DefaultPS_Color = " \
varying		highp vec2	texcoord0;	\
varying		lowp vec4	color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
	lowp vec4 texColor = texture2D(texture0, texcoord0); \
	lowp vec4 temp; \
	temp.a = texColor.a * color.a; \
	temp.r = color.r; \
	temp.g = color.g; \
	temp.b = color.b; \
    gl_FragColor = temp; \
}";

const char* DefaultPS_Tex = " \
varying		highp vec2	texcoord0;	\
varying		lowp vec4	color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
    gl_FragColor = texture2D(texture0, texcoord0); \
}";

const char* DefaultPS_Vtx = " \
varying		highp vec2	texcoord0;	\
varying		lowp vec4	color;		\
\
uniform		sampler2D	texture0;	\
\
void main() \
{ \
    gl_FragColor = color; \
}";
#endif	////IN_EDITOR

const char* DebugPS_Overdraw = " \
void main() \
{ \
    gl_FragColor = vec4( 0.05, 0.05, 0.05, 0.05 ); \
}";
