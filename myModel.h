#ifndef MYMODEL_H
#define MYMODEL_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "lodepng.h"
#include "shaderprogram.h"
#include <vector>
#include <string>

class myModel {
public:
    std::vector<glm::vec4> verts;
    std::vector<glm::vec4> norms;
    std::vector<glm::vec2> texCoords;
    std::vector<unsigned int> indices;
    GLuint tex;

    myModel(const std::string& modelFile, const std::string& textureFile);
private:
    void loadModel(const std::string& file);
    void loadTexture(const std::string& filename);
};

#endif // MODEL_H