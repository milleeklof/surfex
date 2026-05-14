#version 330 core

uniform float minZ;
uniform float maxZ;

in vec3 vNormal;
in float vHeight;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 baseColor;
uniform int renderMode;
uniform float alpha;

vec3 heatmap(float t)
{
    t = clamp(t, 0.0, 1.0);

    vec3 c1 = vec3(0.0, 0.0, 0.5);
    vec3 c2 = vec3(0.0, 0.0, 1.0);
    vec3 c3 = vec3(0.0, 1.0, 0.0);
    vec3 c4 = vec3(1.0, 1.0, 0.0);
    vec3 c5 = vec3(1.0, 0.0, 0.0);

    if (t < 0.25)
        return mix(c1, c2, t / 0.25);
    else if (t < 0.5)
        return mix(c2, c3, (t - 0.25) / 0.25);
    else if (t < 0.75)
        return mix(c3, c4, (t - 0.5) / 0.25);
    else
        return mix(c4, c5, (t - 0.75) / 0.25);
}

void main()
{
    vec3 n = normalize(vNormal);
    vec3 l = normalize(lightDir);

    float diff = max(dot(n, l), 0.0);

    vec3 color;

    if (renderMode == 0)
    {
        color = baseColor;
    }
    else
    {
        float range = max(maxZ - minZ, 0.0001);
        float normalizedHeight = (vHeight - minZ) / range;
        normalizedHeight = clamp(normalizedHeight, 0.0, 1.0);
        color = heatmap(normalizedHeight);
    }

    vec3 ambient = 0.2 * color;
    vec3 diffuse = diff * color;

    FragColor = vec4(ambient + diffuse, alpha);
}
