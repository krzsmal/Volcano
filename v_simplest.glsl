#version 330

// Uniform variables
uniform mat4 P; // Projection matrix
uniform mat4 V; // View matrix
uniform vec4 lp1; // Position of the first light source
uniform vec4 lp2; // Position of the second light source
uniform bool useShading; // Flag for using shading
uniform bool useLight1; // Flag for using light 1
uniform bool useLight2; // Flag for using light 2
uniform bool rotateFirst; // Flag to determine whether to rotate first or translate

// Attributes
in vec4 vertex; // Vertex coordinates in model space
in vec4 normal; // Vertex normal vector in model space
in vec2 texCoord0; // Texture coordinates
in vec3 instanceTranslations;
in vec3 instanceRotationAngles;
in vec3 instanceScale;
in float instanceAlpha;

// Varying variables
out vec4 l1; // Direction vector to light source 1 in eye space.
out vec4 l2; // Direction vector to light source 2 in eye space.
out vec4 n;  // Normal vector in eye space.
out vec4 v;  // Direction vector to the viewer in eye space.
out vec2 iTexCoord0;  // Texture coordinates
out float alpha;

// Function to create a translation matrix
mat4 translate(vec3 t) {
    return mat4(1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                t.x, t.y, t.z, 1.0);
}

// Function to create a rotation matrix by a given angle around a given axis
mat4 rotate(float angle, vec3 axis) {
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    axis = normalize(axis);
    vec3 temp = (1.0 - cosAngle) * axis;

    mat4 rotate;
    rotate[0][0] = cosAngle + temp[0] * axis[0];
    rotate[0][1] = temp[0] * axis[1] + sinAngle * axis[2];
    rotate[0][2] = temp[0] * axis[2] - sinAngle * axis[1];

    rotate[1][0] = temp[1] * axis[0] - sinAngle * axis[2];
    rotate[1][1] = cosAngle + temp[1] * axis[1];
    rotate[1][2] = temp[1] * axis[2] + sinAngle * axis[0];

    rotate[2][0] = temp[2] * axis[0] + sinAngle * axis[1];
    rotate[2][1] = temp[2] * axis[1] - sinAngle * axis[0];
    rotate[2][2] = cosAngle + temp[2] * axis[2];

    mat4 result = mat4(1.0);
    result[0] = vec4(rotate[0]);
    result[1] = vec4(rotate[1]);
    result[2] = vec4(rotate[2]);
    return result;
}

// Function to create a scaling matrix
mat4 scale(vec3 s) {
    return mat4(s.x, 0.0, 0.0, 0.0,
                0.0, s.y, 0.0, 0.0,
                0.0, 0.0, s.z, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

void main(void) {
    // Calculate the model matrix of the instance
    mat4 T = translate(instanceTranslations);
    mat4 Rx = rotate(instanceRotationAngles.x, vec3(1.0, 0.0, 0.0));
    mat4 Ry = rotate(instanceRotationAngles.y, vec3(0.0, 1.0, 0.0));
    mat4 Rz = rotate(instanceRotationAngles.z, vec3(0.0, 0.0, 1.0));
    mat4 S = scale(instanceScale);
    mat4 M;
    if (rotateFirst) {
        M = Rx * Ry * Rz * T * S;
    } else {
        M = T * Rx * Ry * Rz * S;
    }

    // Calculate lighting
    if (useShading) {
        if (useLight1) {
            l1 = normalize(V * lp1 - V * M * vertex); // Calculate direction vector to the first light source in eye space
        } else {
            l1 = vec4(0.0);
        }

        if (useLight2) {
            l2 = normalize(V * lp2 - V * M * vertex); // Calculate direction vector to the second light source in eye space
        } else {
            l2 = vec4(0.0);
        }
        
        v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); // Calculate direction vector to the viewer in eye space
        n = normalize(V * M * normal); // Calculate normal vector in eye space
    }

    iTexCoord0 = texCoord0;
    alpha = instanceAlpha;
    gl_Position = P * V * M * vertex; // Transform the vertex coordinates from model space to screen space
}