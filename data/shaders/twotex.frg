uniform sampler2D texture1;
uniform sampler2D texture2;
varying vec2 texture_coordinate;
varying vec4 color1;
varying vec4 color2;
 
float partOf1 = color1.w;
 
in vec2 texCoord;
 
void main()
{	
	vec4 frag = texture(texture1, texCoord) * vec4(1.0, 1.0, 1.0, partOf1) + texture(texture2, texCoord) * vec4(1.0, 1.0, 1.0, 1.0 - partOf1);
	gl_FragColor = frag;
}
