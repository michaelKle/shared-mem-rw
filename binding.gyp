{
  "targets": [{
    "target_name": "shared_mem_rw",
    "cflags!": [ "-fno-exceptions" ],
    "cflags_cc!": [ "-fno-exceptions" ],
    "sources": [
      "shared-mem-rw.cpp",
	    "addon.cpp",
      "shared-mem.cpp"
    ],
    "include_dirs": [
      "<!@(node -p \"require('node-addon-api').include\")"
    ],
    "conditions": [
        ["OS=='linux'", {
          "link_settings": {
              "libraries": [
                "-lrt"
                ]
          },
        }]
    ],
    'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
  }]
}
