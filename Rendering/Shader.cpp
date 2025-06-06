#include "Shader.h"

Shader::Shader() : m_FilePath(""), m_ID(0)
{

}

Shader::Shader(const std::string& filepath) : m_ID(0)
{
    set(filepath);
}

Shader::~Shader()
{
    glDeleteProgram(m_ID);
}

void Shader::set(const std::string& filepath)
{
    m_FilePath = filepath;
    ShaderProgramSource sources = ParseShader(m_FilePath);
    m_ID = CreateShader(sources);
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::ifstream shader_source(filepath);
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(shader_source, line, '\r'))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else ss[(int)type] << line << '\n';
    }
    //std::cout << ss[0].str() << std::endl;
    //std::cout << ss[1].str() << std::endl;
    std::cout << "Shader source parsed" << std::endl;
    return { ss[0].str(), ss[1].str() };

}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    std::cout << "Shader compiled" << std::endl;
    return id;
}

unsigned int Shader::CreateShader(ShaderProgramSource sources)
{
    unsigned int program = glCreateProgram();
    unsigned int vertex_shader = CompileShader(GL_VERTEX_SHADER, sources.VertexSource);
    unsigned int fragment_shader = CompileShader(GL_FRAGMENT_SHADER, sources.FragmentSource);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void Shader::Bind() const
{
    glUseProgram(m_ID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    glUniform1i(Shader::GetUniformLocation(name), value);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(Shader::GetUniformLocation(name), v0, v1, v2, v3);
}


void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(Shader::GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    int location = glGetUniformLocation(m_ID, name.c_str());
    if (location == -1)
        std::cout << "Warning: uniform '" << name << "' is invalid" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}

