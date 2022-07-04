#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"
#include "shader_s.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
unsigned int TextureFromFile(const char* path, const std::string& directory);

class Model {
public:
    //Model属性
    std::vector<Texture> textures_loaded;//记录加载过的纹理，节省开销
    std::vector<Mesh> meshes;//Mesh数组
    std::string directory;//

	//构造函数
    Model(std::string const& path)
    {
        loadModel(path);
    }

    void Draw(Shader shader) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);//绘制每一个Mesh
    }
private:
    

    
    //加载模型函数
    void loadModel(std::string const& path)
    {
        // 声明一个Importer
        Assimp::Importer importer;
        
        //创建scene场景
        //path：路径
        //aiProcess_Triangulate：转换为三角形网格处理
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        //报错
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }

        //检索文件路径
        directory = path.substr(0, path.find_last_of('/'));

        //递归处理scene的根节点(处理每一个节点的网格数据)
        processNode(scene->mRootNode, scene);

    }

    //递归处理所有网格
    void processNode(aiNode* node, const aiScene* scene) {
        // 处理节点所有的网格（如果有的话）
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            //获取根节点的所有网格
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // 接下来对它的子节点重复这一过程
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    //将aiMesh转化为自己的网格对象Mesh
    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        //获取、处理网格顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // 处理顶点位置、法线和纹理坐标
            glm::vec3 Pos,Norm,Tex;
            Pos.x = mesh->mVertices[i].x;
            Pos.y = mesh->mVertices[i].y;
            Pos.z = mesh->mVertices[i].z;
            vertex.Position = Pos;

            if (mesh->HasNormals()) {
                Norm.x = mesh->mNormals[i].x;
                Norm.y = mesh->mNormals[i].y;
                Norm.z = mesh->mNormals[i].z;
                vertex.Normal = Norm;
            }

            if (mesh->mTextureCoords[0]) {//存在纹理坐标
                glm::vec2 Tex;
                Tex.x = mesh->mTextureCoords[0][i].x;
                Tex.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = Tex;
            }
            else {
                vertex.TexCoords = glm::vec2(0, 0);
            }
            vertices.push_back(vertex);
        }
        // 处理索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // 处理材质
        if (mesh->mMaterialIndex >= 0)
        {
            //从场景中获取aiMaterial对象
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            //加载漫反射贴图、获取纹理
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material,aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            //加载镜面光贴图、获取纹理
            std::vector<Texture> specularMaps = loadMaterialTextures(material,aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(vertices, indices, textures);

    }

    //获取材质纹理
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {   // 如果纹理还没有被加载，则加载它
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // 添加到已加载的纹理中
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif

