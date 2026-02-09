if executable('docker')
  augroup LspClangdDocker
    autocmd!
    autocmd User lsp_setup call lsp#register_server({
          \ 'name': 'clangd-docker',
          \ 'cmd': {server_info -> [
          \   'docker',
          \   'exec',
          \   '-i',
          \   'minic-dev',
          \   'clangd',
          \   '--compile-commands-dir=build',
          \   '--background-index'
          \ ]},
          \ 'allowlist': ['c', 'cpp', 'objc', 'objcpp'],
          \ })
  augroup END
endif

let g:lsp_settings_filetype_cpp = ['clangd-docker']
