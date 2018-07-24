Model Loader
============

The goal of this file is loading mesh data. The projhect is related to my previous [Maya Exporter](https://github.com/Cabrra/Maya-Exporter); this time, is a file reader to load the exported meshes.
The format:
	The length of the name of the mesh, including the null terminator (4 bytes - unsigned int).
	The mesh name, including the null terminator. ("length" bytes).
	The number of textures (4 bytes - unsigned int).
	// For each texture
	The length of the texture name, including the null terminator (4 bytes - unsigned int).
	The texture name, including the null terminator("length" bytes).
	The number of unique vertices (4 bytes - unsigned int).
	// For Each vertex
	Position (3 floats)
	Normal (3 floats)
	Texture coordinates (2 floats)
	The number of triangles (4 bytes - unsigned int)
	// For each triangle
	The three vertex indices (3 unsigned int)
	
To use the loader just run the executable and load the test meshes.

## Built With

* [Visual Studio](https://visualstudio.microsoft.com/)	- For C++ development


## Contributing

Please read [CONTRIBUTING.md](https://github.com/Cabrra/Contributing-template/blob/master/Contributing-template.md) for details on our code of conduct, and the process for submitting pull requests to me.


## Authors

* **Jagoba "Jake" Marcos** - [Cabrra](https://github.com/Cabrra)


## License

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details


## Acknowledgments

* Full Sail University - Game Development Department