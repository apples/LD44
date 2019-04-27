precision mediump float;

varying vec2 v_texcoord;
varying vec3 v_normal;

uniform sampler2D s_texture;
uniform vec3 cam_forward;
uniform vec4 tint;
uniform float hue;
uniform float saturation;
uniform vec3 sky_dir;
uniform vec3 sky_color;
uniform vec3 ambient_color;
uniform bool enable_lighting;

const mat4 dither_mask = mat4(
    1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
    13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
    4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
    16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0);

float get_dither_threshold()
{
    ivec2 dither_coord = ivec2(mod(gl_FragCoord.xy, 4.0));

    vec4 dither_mask_column = vec4(0);

    if (dither_coord.x == 0) dither_mask_column = dither_mask[0];
    if (dither_coord.x == 1) dither_mask_column = dither_mask[1];
    if (dither_coord.x == 2) dither_mask_column = dither_mask[2];
    if (dither_coord.x == 3) dither_mask_column = dither_mask[3];

    float dither_threshold = 0.0;

    if (dither_coord.y == 0) dither_threshold = dither_mask_column[0];
    if (dither_coord.y == 1) dither_threshold = dither_mask_column[1];
    if (dither_coord.y == 2) dither_threshold = dither_mask_column[2];
    if (dither_coord.y == 3) dither_threshold = dither_mask_column[3];

    return dither_threshold;
}

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
    vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    if (dot(v_normal, cam_forward) > 0.0) discard;

    vec4 color = texture2D(s_texture, v_texcoord) * tint;

    float dither_threshold = get_dither_threshold();

    if (color.a < dither_threshold) discard;

    vec3 hsv = rgb2hsv(color.rgb);

    hsv.x += hue;
    hsv.y *= saturation;

    color.rgb = hsv2rgb(hsv);
    color.a = 1.0;

    if (enable_lighting) {
        vec3 diffuse = 0.6 * max(dot(v_normal, sky_dir), 0.0) * sky_color;
        vec3 ambient = 0.4 * ambient_color;
        color.rgb = color.rgb * (ambient + diffuse);
    }

    gl_FragColor = color;
}
