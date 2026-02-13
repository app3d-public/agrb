# App3D Graphics Rendering Backend

**Agrb** is a lightweight Vulkan-oriented rendering backend for the App3D ecosystem.  
It is not a full abstraction layer, but rather a collection of utilities, helpers, and patterns that simplify common Vulkan tasks and make low-level GPU resource management more ergonomic.

## Building

### External packages
These are system libraries that must be available at build time:
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)

### Bundled submodules
The following dependencies are included as git submodules and must be checked out when cloning:

- [acbt](https://github.com/app3d-public/acbt)
- [acul](https://github.com/app3d-public/acul)
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)

### Supported compilers:
- GNU GCC
- Clang

### Supported OS:
- Linux
- Microsoft Windows

### Cmake options:
- `BUILD_TESTS`: Enable testing
- `ENABLE_COVERAGE`: Enable code coverage

## Tools

AGRB includes additional tooling for shader build/deploy flow:
- `shader_builder.py`: generates shader command files, headers, and depfiles from YAML.
  See `modules/agrb/tools/README.md`.
- `agrb_s2u`: packs compiled `.spv` files into `.umlib` shader libraries.
  See `modules/agrb/tools/s2u/README.md`.

## License
This project is licensed under the [MIT License](LICENSE).

## Contacts
For any questions or feedback, you can reach out via [email](mailto:wusikijeronii@gmail.com) or open a new issue.
