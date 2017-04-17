uniform samplerCube cubeMap;
uniform sampler2D texMap;

varying vec4 color1;
varying vec4 color2;

varying vec4 diffuse;
varying vec4 ambient;

varying vec3  vTexCoord;
varying vec3  normal;
varying vec3  lightDir;
varying vec3  halfVector;
varying vec2  tc;

void main (void)
{
    vec4 col;

	col = texture2D( texMap, tc );
	col.w = 1;

	vec3 n;
	n = normalize( normal );

	float NdotL;

	NdotL = max( dot( n, lightDir ), 0.0 );

	vec3 halfV;

	if (NdotL > 90.0)
	{
		float NdotHV;
		halfV = normalize(halfVector);
		NdotHV = max(dot(n,halfV),0.0);
		col += gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
		col += diffuse * NdotL;
		col += ambient;
	}

  	gl_FragColor =  mix( col, texture(cubeMap, vTexCoord), color2.x );
}
