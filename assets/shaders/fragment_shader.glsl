#version 330 core
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 lightPos2;

uniform vec3 lightColor;
uniform vec3 lightColor2;

uniform vec3 viewPos;  // camera position

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // -------------------------
    // Light 1
    // -------------------------
    vec3 lightDir1 = normalize(lightPos - FragPos);

    // Diffuse
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * lightColor;

    // Specular
    vec3 reflectDir1 = reflect(-lightDir1, norm);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 32.0);
    vec3 specular1 = spec1 * lightColor;

    // Ambient
    vec3 ambient1 = 0.1 * lightColor;

    // -------------------------
    // Light 2
    // -------------------------
    vec3 lightDir2 = normalize(lightPos2 - FragPos);

    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;

    vec3 reflectDir2 = reflect(-lightDir2, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32.0);
    vec3 specular2 = spec2 * lightColor2;

    vec3 ambient2 = 0.1 * lightColor2;

    // -------------------------
    // Final result
    // -------------------------
    vec3 ambient = ambient1 + ambient2;
    vec3 diffuse = diffuse1 + diffuse2;
    vec3 specular = specular1 + specular2;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result, 1.0);
}
