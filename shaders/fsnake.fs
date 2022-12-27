#version 330 core
out vec4 fragColor;
in vec3 outNormal;
const vec3 lightDir = vec3(10.0,-10.0,-10.0);
const vec3 lightColor = vec3(1.0,1.0,1.0);
const vec3 ambientColor = vec3(1.0,1.0,1.0);
uniform vec3 color;
const float aStrength = 0.3;
void main()
{
	vec3 tNormal = normalize(outNormal);
	vec3 lDir = normalize(-lightDir);
	float dStrength = max(dot(tNormal,lDir),0.0);
	vec3 diffuse = dStrength * lightColor * color;
	vec3 ambient = aStrength * ambientColor * color;
	vec3 oC = diffuse + ambient;
	fragColor = vec4(oC,1.0);
}
