bool Import3DFromFile( const aiScene** sc, const std::string& pFile);
std::vector<struct MyMesh> createMeshFromAssimp(const aiScene* sc, std::string filename, GLuint*& textureIds);