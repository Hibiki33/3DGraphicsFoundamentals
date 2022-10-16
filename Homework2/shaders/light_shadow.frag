#version 330 core

#define PI 3.1415926
#define EPSILON 0.000001

out vec4 FragColor;

uniform vec3 point_light_pos;
uniform vec3 point_light_radiance;
uniform sampler2D shadowmap;				// ��Ӱ��ͼ
uniform bool have_shadow;

//TODO: 2.2 ��ӱ�����������ռ�ӳ�䵽��Ӱӳ��ƽ��ռ�
uniform mat4 lightSpaceMatrix;

uniform float ambient;
uniform float specular;
uniform sampler2D color_texture;

uniform vec3 camera_pos;

in VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    vec3 Normal;
} vs_out;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;// �����пռ�����ķ�Χ-w��wתΪ-1��1
																				// �任��[0,1]�ԱȽ������ͼ�����
    float closestDepth = texture(shadowmap, projCoords.xy).r;		// �������
    float currentDepth = projCoords.z;								// �������
	vec2 texelSize = 1.0 / textureSize(shadowmap, 0);				// ��������Ƭ�εĴ�С
	float bias = 0.0005;				// ���ƫ��ֵ ����ó� 0 < bias < 0.001 �ǱȽϺ��ʵ�ƫ����
										// ��bias == 0ʱ ����Shadow Acne
										// ��bias == 0.005fʱ ������Ӱ��ʧ
	float shadow = 0.0;
	// ��Ƭ�����ܶ������ͼ������ƽ�� �Խ��;�ݿ�
	for(int x = -2; x <= 2; x++)
	{
		for(int y = -2; y <= 2; y++)
		{
			float pcfDepth = texture(shadowmap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 25.0;						// ��������Χ��5 * 5��Ƭ�Σ����أ� ƽ��
    return shadow;
}

void main() {
	vec3 color = texture(color_texture, vs_out.TexCoord).rgb;
	//TODO: 2.2 ���Ƚ���һ�����light_dn.frag��Phong shading ����ճ�������Ȼ�������Ӱ
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	vec3 lightColor = point_light_radiance / 255.0;
	vec3 fragPos = vs_out.WorldPos;
	vec3 normal = normalize(vs_out.Normal);
	vec3 lightPos = point_light_pos;
	vec3 viewPos  = camera_pos;
    vec3 lightDirect = normalize(lightPos - fragPos);
    vec3 viewDirect = normalize(viewPos - fragPos);
	vec3 A = ambient * lightColor;
    float diffColor = max(dot(normal, lightDirect), 0.0);
    vec3 D = diffColor * lightColor;
    vec3 reflectDirect = reflect(-lightDirect, normal);  
    float specColor = pow(max(dot(viewDirect, reflectDirect), 0.0), 32);
    vec3 S = specular * specColor * lightColor;

	// ������Ӱ
	float shadow = 0.0;
	if(have_shadow)
	{
		shadow = ShadowCalculation(fragPosLightSpace); 
	}

	vec3 Lo = color * (A + (1.0 - shadow) * (D + S));
	FragColor = vec4(Lo, 1.0);
}
