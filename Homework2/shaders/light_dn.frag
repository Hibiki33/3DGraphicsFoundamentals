#version 330 core

#define PI 3.1415926
#define EPSILON 0.000001

out vec4 FragColor;							// ���Ƭ����ɫ����ɫ

uniform vec3 point_light_pos;				// ���Դλ��
uniform vec3 point_light_radiance;			// ����

uniform float ambient;						// ������ǿ��
uniform sampler2D color_texture;			// ������ɫ
uniform sampler2D normalmap;				// ������ͼ
uniform bool have_normal_and_displacement;	// �Ƿ���Ϸ�����ͼ���û���ͼ

uniform vec3 camera_pos;					// �ӽ�����λ��

in VS_OUT {
    vec3 WorldPos;							// Ƭ�ε���������
    vec2 TexCoord;							// Ƭ�ε���������
    mat3 TBN;								// ����T��B��Ϊ0 N��Ƭ�εķ�����
} vs_out;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main() {
	vec3 albedo = texture(color_texture, vs_out.TexCoord).rgb;
	// TODO: 2.0 Phong Shading
	// ������������ÿ��������Ϊ0.0-1.0֮��ĸ����� ת��Ϊ������ɫ
	vec3 lightColor = point_light_radiance / 255.0;
	vec3 fragPos = vs_out.WorldPos;
	// ��ó����������Ĺ�һ���ķ�����
	vec3 normal = normalize(vs_out.TBN[2]);
	if (dot((camera_pos - vs_out.WorldPos), normal) < -EPSILON) 
	{
		normal = -normal;
	}
	vec3 lightPos = point_light_pos;
	vec3 viewPos  = camera_pos;
    vec3 lightDirect = normalize(lightPos - fragPos);
    vec3 viewDirect = normalize(viewPos - fragPos);
	
	if(have_normal_and_displacement)
	{
		// TODO: 2.1 ������ͼ 
		lightPos = transpose(vs_out.TBN) * lightPos;
		viewPos = transpose(vs_out.TBN) * viewPos;
		fragPos = transpose(vs_out.TBN) * fragPos;

        lightDirect = normalize(lightPos - fragPos);
        viewDirect = normalize(viewPos - fragPos);

        normal = texture(normalmap, vs_out.TexCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0); // ����������ת��Ϊ��Χ[-1,1] or normalize(normal * 0.5 + 0.5)
		if (dot((viewPos - fragPos), normal) < -EPSILON) 
		{
			normal = -normal;
		}
	}

	// ������
	vec3 A = ambient * lightColor;
	// ������ 
    float diffColor = max(dot(normal, lightDirect), 0.0);
    vec3 D = diffColor * lightColor;
	// �߹�
	float specular = 0.5;
    vec3 reflectDirect = reflect(-lightDirect, normal);  
    float specColor = pow(max(dot(viewDirect, reflectDirect), 0.0), 32);
    vec3 S = specular * specColor * lightColor;

	// vec3 Lo = albedo; (orig)
	FragColor = vec4((A + D + S) * albedo, 1.0);
}

