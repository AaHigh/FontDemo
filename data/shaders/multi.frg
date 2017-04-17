uniform sampler2D my_color_texture;
varying vec2 texture_coordinate;
varying vec4 color1;
varying vec4 color2;

void main()
{
	vec4 frag = texture2D( my_color_texture, texture_coordinate );
	// assumes monochrome texture and mixes based on red component only
	vec4 blend = mix( color2, color1, frag.x );
	blend.w = frag.w * color1.w; // modulate incoming alpha w/texture alpha
    gl_FragColor = blend;
}
