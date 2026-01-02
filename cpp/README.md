## Building Image

Since the image provided does not have clangd installed, so we need to either install manually or make a custom one that derives from the provied `compiler-dev` environment. Clearly, the second one is easier to reuse overtime, so I will go with the second approach.

```sh
cat > Dockerfile.clangd <<'EOF'
FROM maxxing/compiler-dev
RUN apt-get update && apt-get install -y clangd && rm -rf /var/lib/apt/lists/*
EOF
docker build -t minic-dev-clangd -f Dockerfile.clangd .
```

## Running the Image

Then, we need to run it. In order to have LSP working, I need to be using the same absolute path as neovim clangd. Hence, we bind mount it.

```sh
# Uncomment the following line to remove anything that was named the same.
# docker rm -f minic-dev 2>/dev/null
docker run -d --name minic-dev \
  -v "$PWD:$PWD" -w "$PWD" \
  minic-dev-clangd bash -lc 'sleep infinity'
```

## Setup `clangd`

Most of the work is already done as we have already downloaded it when building the image. However, we do need to let neovim know that we are using the clangd inside the image. But first, let's validate that we do have `clangd` installed in the image.

```sh
docker exec -it minic-dev clangd --version
```

Then, we need to create `.nvim.lua` for a local setup for neovim.

```lua
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
```

Note that you need to set the following flags for neovim to allow local config.

```lua
vim.o.exrc = true
vim.o.secure = true
```

## Compiling

Compiling using cmake:

```sh
docker exec -it minic-dev cmake --build build -j
```

Then, run the compiled program with,

```sh
docker exec -it minic-dev ./build/compiler -koopa example/hello.c -o hello.koopa
```
