{
  "targets": [
    {
        "target_name": "NodeJsBinding",
        'defines': [
            '__DEBUG=1',
            'node_gyp=1'
        ],
        'include_dirs': [
            '../src/Common2',
        ],
        "sources": [ "../api/nodejs_binding/nodejs_binding.cpp" ]
    }
  ]
}