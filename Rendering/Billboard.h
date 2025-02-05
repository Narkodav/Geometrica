#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Rendering/Shader.h"

class Billboard
{
private:
    GLuint VAO, VBO, textureID;
    glm::vec3 position;

    bool isSet = 0;

public:

    ~Billboard() {
        if (!isSet)
            return;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void set()
    {
        isSet = 1;
        // Vertex data for a quad (2 triangles)
        float vertices[] = {
            // positions        // texture coords
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
             0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f
        };

        // Generate and bind VAO/VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    void render(Shader& shader, unsigned int textureIndex, const glm::mat4& view, const glm::mat4& projection) {
        shader.Bind();

        // Calculate billboard rotation to face camera
        glm::vec3 cameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
        glm::vec3 cameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);

        // Create model matrix for the billboard
        glm::mat4 model = glm::mat4(1.0f);
        model[0] = glm::vec4(cameraRight, 0.0f);
        model[1] = glm::vec4(cameraUp, 0.0f);
        model[2] = glm::vec4(glm::cross(cameraRight, cameraUp), 0.0f);
        model[3] = glm::vec4(position, 1.0f);

        // Set uniforms
        glUniformMatrix4fv(shader.GetUniformLocation("u_modelTransform"), 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(shader.GetUniformLocation("u_viewTransform"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(shader.GetUniformLocation("u_projectionTransform"), 1, GL_FALSE, &projection[0][0]);
        glUniform1ui(shader.GetUniformLocation("u_TextureIndex"), textureIndex);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Draw billboard
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void setPosition(const glm::vec3& pos) {
        position = pos;
    }

    void setTexture(GLuint texID) {
        textureID = texID;
    }
};

