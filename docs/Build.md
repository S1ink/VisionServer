# Building VisionServer
__VisionServer uses Make to automate building. The included makefile supports automatic platform detection and provides a few different options and targets.__

## Targets
| Target | Description | Example Usage |
| - | - | - |
| `shared` | This is the default target - it builds the shared library libvs3407.so. | `make shared` or just `make` |
| `static` | This builds a static library (archive) of all sources - libvs3407.a. Note that the correct libraries will need to be linked to when using this form. | `make static` |
| `clean` | This deletes all outputs and intermediate files, and is sometimes required when building after changing paramters (ex. debug to release). | `make clean` |
| `rebuild` | This calls `rebuild-shared`, and is only included to provide congruency for `shared` being the default build option. | `make rebuild` |
| `rebuild-shared` | This cleans all outputs and then builds the shared library. | `make rebuild-shared` |
| `rebuild-static` | Clean all outputs and then build the static library. | `make rebuild-static` |

---
## Options (Reassignable Variables)
| Variable | Description | Options | Default Option | Example Usage |
| - | - | - | - | - |
| `OPT` | The OPTimization level that sources are compiled with. | `debug` : -g -Og -D__DEBUG__ | `release` | `make OPT=debug` |
| ^ | ^ | `release` : -O3 -D__RELEASE__ | ^ | ^ |
| `TFLITE` | Defines whether or not TfLite (and edgeTPU) libs are linked with, and the corresponding code that uses these are included. This is useful if ML vision will not be used and it is undesired to deal with manually updating these libraries (they are not included by default on WPILibPi) | `true`/`yes`/`y`/`include` : TfLite included | `true` | `make TFLITE=exclude` |
| ^ | ^ | `false`/`no`/`n`/`exclude` : TfLite not included | ^ | ^ |
| `ADEFINES` | Any additional defines to be declared when compiling. For multiple values, separate by spaces and surround with quotes. Note that "-D" is automatically appended to each define, and this is not required for input. | n/a | n/a | `make ADEFINES="REMOVE_DISCONNECTED_CAMERAS COMPENSATE_ON_STARTUP"` |

__Other internally modifyable and more general options include:__
| Variable | Description |
| - | - |
| `CROSS_PREFIX` | The prefix for the cross-compiler. |
| `CXX` | The name of the compiler. |
| `AR` | The name of the archiving tool. |
| `STD` | What version of the standard library to use. |
| `*_DIR` | These are self-explainatory, see the makefile for all options. |

---
## Linking and Required Libraries
__All required libraries are included along with the source and are updated automatically using a GitHub Action. In case any of the dependencies need to be referenced, below is a list of all required libraries and how they were sourced.__
| Library | Sourcing Method / Notes | Link(s) |
| - | - | - |
| WPILib (built for raspbian, contains OpenCV) | Extracted from the "cpp-multiCameraServer" example, which is located on the latest WPILibPi release. __NEW: Built using Github Actions in order to include the Aruco module for OpenCV.__ | https://github.com/wpilibsuite/WPILibPi/releases/latest |
| ^ | ^ | https://github.com/S1ink/WPILibPi-Mod-Build/releases/latest |
| Pigpio | Built from source. | https://github.com/joan2937/pigpio |
| Tensorflow Lite | Built from source __using Bazel__. Headers are extracted using this command: ``find ./tensorflow -name '*.h' \| cpio -pdm OUTPUT_DIR`` | https://github.com/tensorflow/tensorflow |
| ^ | ^ | https://www.tensorflow.org/lite/guide/build_arm#cross-compilation_for_arm_with_bazel |
| EdgeTPU driver | Built from source using the included dockerfile and Bazel. Note that in order to function properly with TfLite, the `workspace.bzl` file needs to be configured with the same tensorflow commit as was used to build tensorflow lite. Also note that the output libs are stripped, and this causes them to fail to be linked during cross-compilation. I solved this problem by removing all instances of `$``(STRIPPED_SUFFIX)` within the makefile using the command ``sed -i 's/$````(STRIPPED_SUFFIX)//' ./Makefile`` | https://github.com/google-coral/libedgetpu |
| Flatbuffers (headers for TfLite) | The automation script uses this command (while within the tensorflow repo directory): ``wget -O flatbuffers.tar.gz $````(grep 'urls = tf_mirror_urls(".*"' third_party/flatbuffers/workspace.bzl \| cut -d \" -f2)`` . The headers are then located under `flatbuffers/include/flatbuffers/` | http://github.com/google/flatbuffers |
| Abseil (headers for TfLite) | The automation script uses this command (while within the tensorflow rep directory): ``wget -O absl.tar.gz https://github.com/abseil/abseil-cpp/archive/$````(grep 'ABSL_COMMIT = ".*"' third_party/absl/workspace.bzl \| cut -d \" -f2).tar.gz`` . The headers can then be extracted using: ``find ./absl -name '*.h' \| cpio -pdm OUTPUT_DIR`` | https://github.com/abseil/abseil-cpp |

When linking statically to VisionServer, it is also required to link to many of these libraries. For a complete list it is best to reference the makefile.