#shader vertex#version 330 core//2d coordslayout(location = 0) in vec2 v_position; layout(location = 1) in vec2 v_texCoordIn;out vec2 v_texCoord;void main(){   gl_Position = vec4(v_position, 0, 1);   v_texCoord = v_texCoordIn;};#shader fragment#version 330 corelayout(location = 0) out vec4 FragColor;in vec2 v_texCoord;uniform sampler2D u_Texture;void main(){	FragColor = texture(u_Texture, v_texCoord);};