#version 330

out vec4 pixelColor; // Output variable. Almost final pixel color.

uniform sampler2D textureMap0; // Texture
uniform bool useShading; // Flag for using shading
uniform bool useLight1; // Flag for using light 1
uniform bool useLight2; // Flag for using light 2
uniform vec4 lightColor1; // Color of the first light
uniform vec4 lightColor2; // Color of the second light

// Varying variables
in vec4 n; // Direction vector to the viewer in eye space.
in vec4 l1; // Direction vector to light source 1 in eye space.
in vec4 l2; // Direction vector to light source 2 in eye space.
in vec4 v; // Direction vector to the viewer in eye space.
in vec2 iTexCoord0; // Texture coordinates
in float alpha;

void main(void) {
    vec4 kd = texture(textureMap0, iTexCoord0); // Retrieve color from texture 
    vec4 finalColor;

    if (useShading) {
        // Normalize direction vectors
        vec4 ml1 = normalize(l1);
        vec4 ml2 = normalize(l2);
        vec4 mn = normalize(n);
        vec4 mv = normalize(v);

        // Calculate reflection vectors for the lights
        vec4 mr1 = reflect(-ml1, mn);
        vec4 mr2 = reflect(-ml2, mn);

        // Determine light reflection intensity on the surface
        vec4 ks1 = vec4(0.25, 0.25, 0.25, 1);
        vec4 ks2 = vec4(0.25, 0, 0, 1);

        // Ambient light
        vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1.0);
        vec4 ambientTerm = ambientColor * kd;
        
        vec4 color1 = vec4(0.0);
        vec4 color2 = vec4(0.0);

        if (useLight1) {
            float nl1 = 0.5 * clamp(dot(mn, ml1), 0, 1);                                // Calculate the diffuse component
            float rv1 = 0.4 * pow(clamp(dot(mr1, mv), 0, 1), 50);                       // Calculate the specular component
            color1 = (vec4(kd.rgb * nl1, kd.a) + vec4(ks1.rgb * rv1, 0)) * lightColor1; // Calculate the final pixel color relative to the first light
        }

        if (useLight2) {
            float nl2 = 1 * clamp(dot(mn, ml2), 0, 1);
            float rv2 = 1 * pow(clamp(dot(mr2, mv), 0, 1), 50);
            color2 = (vec4(kd.rgb * nl2, kd.a) + vec4(ks2.rgb * rv2, 0)) * lightColor2; // Calculate the final pixel color relative to the second light
        }

        // Final calculation of the pixel color based on both light sources and ambient lighting
        finalColor = color1 + color2 + ambientTerm;
        finalColor = clamp(finalColor, 0.0, 1.0);
    } else {
        finalColor = kd;
    }

    finalColor.a = finalColor.a * alpha; // Set transparency 
    pixelColor = finalColor;             // Final pixel color
}