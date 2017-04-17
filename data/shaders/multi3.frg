uniform sampler2D my_color_texture;
varying vec2 texture_coordinate;
varying vec4 color1;
varying vec4 color2;
uniform vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 black = vec4(0.0, 0.0, 0.0, 1.0);

void main()
{
	vec4 frag = texture2D( my_color_texture, texture_coordinate );
	// assumes monochrome texture and mixes based on red component only
	vec4 c3 = vec4( color1.x / 2, color1.y / 2, color1.z / 2, color2.w );
	vec4 c1 = mix( c3, color1, frag.z );
//	c3 = vec4( color2.x / 2, color2.y / 2, color2.z / 2, color2.w );
	vec4 c2 = mix( c3, color2, frag.y );
	vec4 blend = mix( c1, c2, 0.5 );
	blend = mix( blend, white, frag.x );
	blend.w = frag.w * color1.w; // modulate incoming alpha w/texture alpha
    gl_FragColor = blend;
}
