#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D displacementmap;              // �û���ͼ
uniform float displacement_coefficient;         // �û�ϵ��
uniform bool have_normal_and_displacement;      // �Ƿ���뷨�ߺ��û���ͼ

void main()
{
    
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.WorldPos = worldPos.xyz / worldPos.w;
	
    vs_out.TexCoord = aTexCoord;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = vec3(0);
	vec3 B = vec3(0);

    if(have_normal_and_displacement)
    {
        // TODO: 2.1 ����Displacement������WorldPos
        T = normalize(normalMatrix * aTangent);
        B = normalize(cross(T, N));
        float Displacement = texture(displacementmap, vs_out.TexCoord).r;
        vs_out.WorldPos += N * Displacement * displacement_coefficient;
        worldPos = vec4(vs_out.WorldPos, 1.0);
    }

    if(have_normal_and_displacement)
    {
        // TODO: 2.1 ����ģ������������ϵ�ķ��ߣ��Ӷ������ڶ�����ɫ�����÷�����ͼ����
        float c_step = 1.0;

        float tangent_height = texture(displacementmap, vec2(aTangent.x + c_step, aTangent.y)).r;
        vec3 tangent_pos = vs_out.WorldPos + T * c_step + N * tangent_height * displacement_coefficient;
        T = normalize(tangent_pos - vs_out.WorldPos);

        float bitangent_height = texture(displacementmap, vec2(aTangent.x, aTangent.y + c_step)).r;
        vec3 bitangent_pos = vs_out.WorldPos + B * c_step + N * bitangent_height * displacement_coefficient;
        B = normalize(bitangent_pos - vs_out.WorldPos);

        N = normalize(cross(B, T));
        B = normalize(cross(T, N));
    }

	vs_out.TBN = mat3(T, B, N);
    gl_Position = projection * view * worldPos;
}
