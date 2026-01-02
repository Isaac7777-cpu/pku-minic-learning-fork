vim.lsp.config("clangd", {
	cmd = {
		"docker",
		"exec",
		"-i",
		"minic-dev",
		"clangd",
		"--compile-commands-dir=build",
		"--background-index",
	},
})
