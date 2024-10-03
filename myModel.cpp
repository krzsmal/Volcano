#include "myModel.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// constructor
myModel::myModel(const std::string& modelFile, const std::string& textureFile) {
    loadModel(modelFile);
    loadTexture(textureFile);
}

// function to load the model from an fbx file
void myModel::loadModel(const std::string& file) {
    Assimp::Importer importer;

    // load the file
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    if (!scene) {
        std::cerr << importer.GetErrorString() << std::endl;
        return;
    }

    aiMesh* mesh = scene->mMeshes[0]; // get the first mesh

    // iterate over all vertices from the mesh and store coordinates, normals, and texture coordinates for each
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        aiVector3D vertex = mesh->mVertices[i];
        verts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

        aiVector3D normal = mesh->mNormals[i];
        norms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

        aiVector3D texCoord = mesh->mTextureCoords[0][i];
        texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
    }

    // for each triangle in the mesh, store the vertex indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
}

void myModel::loadTexture(const std::string& filename) {
    glActiveTexture(GL_TEXTURE0); // set the active texture unit
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filename);
    if (error != 0) {
        std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
        return;
    }

    glGenTextures(1, &tex); // generate a texture
    glBindTexture(GL_TEXTURE_2D, tex); // bind the generated texture to the texture unit
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data()); // load the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // set linear interpolation for minifying
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // set linear interpolation for magnifying
}
