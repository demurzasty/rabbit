#include <rabbit/rabbit.hpp>

using namespace rb;

static const char data[] =
R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
Nunc semper lacus eu feugiat sagittis. Nullam in nunc felis. 
Aliquam erat volutpat. Donec consequat tellus eu elit vestibulum, 
ut sodales mi interdum. Pellentesque nunc leo, rutrum eget euismod non, 
viverra pretium diam. Cras ultricies sapien lectus, at eleifend 
lacus efficitur vitae. Etiam vehicula sagittis dolor in volutpat. 
Integer interdum orci rutrum, facilisis nunc in, laoreet lectus. 
Integer ac urna id leo sodales tristique. Vivamus vel luctus neque. 
Cras ac libero quis mi suscipit pretium. Fusce quam quam, fermentum 
eu dignissim ac, aliquam id enim. Integer vitae nisl pharetra, consequat 
dui non, scelerisque justo. Curabitur pretium dolor ac ultrices bibendum.)";

int main(int argc, char* argv[]) {
    compressor compressor;
    std::vector<std::uint8_t> compressed_data = compressor.compress<char>(data);
    
    println("data size: {} compressed size: {}", sizeof(data), compressed_data.size());

    std::vector<char> uncompressed_data = compressor.uncompress<char>(sizeof(data), compressed_data);

    println("uncompressed data: \n{}", uncompressed_data.data());
}
