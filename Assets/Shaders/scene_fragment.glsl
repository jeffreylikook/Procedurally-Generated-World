#version 330 core

const float PI = 3.1415926535897932384626433832795;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;

uniform vec3 object_color;

const float shading_ambient_strength    = 0.3;
const float shading_diffuse_strength    = 0.9;
const float shading_specular_strength   = 0.3;

uniform vec3 view_position;

uniform sampler2D shadow_map;
uniform sampler2D textureSampler;

in vec3 fragment_position;
in vec4 fragment_position_light_space;
in vec3 fragment_normal;

in vec4 gl_FragCoord;
in vec2 vertexUV;
out vec4 result;

vec3 ambient_color(vec3 light_color_arg) {
    return shading_ambient_strength * light_color_arg;
}

vec3 diffuse_color(vec3 light_color_arg, vec3 light_position_arg) {
    vec3 light_direction = normalize(light_position_arg - fragment_position);
    return shading_diffuse_strength * light_color_arg * max(dot(normalize(fragment_normal), light_direction), 0.0f);
}

vec3 specular_color(vec3 light_color_arg, vec3 light_position_arg) {
    vec3 light_direction = normalize(light_position_arg - fragment_position);
    vec3 view_direction = normalize(view_position - fragment_position);
    vec3 reflect_light_direction = reflect(-light_direction, normalize(fragment_normal));
    return shading_specular_strength * light_color_arg * pow(max(dot(reflect_light_direction, view_direction), 0.0f),32);
}

float shadow_scalar() {
    // this function returns 1.0 when the surface receives light, and 0.0 when it is in a shadow
    // perform perspective divide
    vec3 normalized_device_coordinates = fragment_position_light_space.xyz / fragment_position_light_space.w;
    // transform to [0,1] range
    normalized_device_coordinates = normalized_device_coordinates * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragment_position_light_space as coords)
    float closest_depth = texture(shadow_map, normalized_device_coordinates.xy).r;
    // get depth of current fragment from light's perspective
    float current_depth = normalized_device_coordinates.z;
    // check whether current frag pos is in shadow
    float bias = 0.005;  // bias applied in depth map: see shadow_vertex.glsl
    return ((current_depth - bias) < closest_depth) ? 1.0 : 0.0;
}

void main()
{
    vec3 ambient = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);
	vec4 textureColor = texture( textureSampler, vertexUV );

    //The shadows are disabled because they are broken.
    float scalar = 1.0f;
    ambient = ambient_color(light_color);
    diffuse = scalar * diffuse_color(light_color, light_position);
    specular = scalar * specular_color(light_color, light_position);
    
    vec3 color = (specular + diffuse + ambient) * object_color;
	result = vec4(color, 1.0f) * textureColor;
	
}

