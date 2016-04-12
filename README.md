__Assume-se para este tutorial que o computador configurado possui ambiente Linux e GCC instalado.__

Compressor Distribuído (MPI) e Paralelo (PThreads) de arquivos (Na visão de usuário):
========

```
* Compressor (Permite a compressão de arquivos utilizando similaridade de coocorrências).
```

Compilando o código fonte do Servidor/Cliente (GNU GCC):
-----------
```
mpicc mcompressor.c -lpthread -o compressor
```

Executando o compressor
-----------
```
- mpirun -f host_file ./compressor input_file.doc output_file.doc
```

Compressor (Na visão do desenvolvedor):
========

Visão geral do repositório:
-----------
1. mcompressor.c: Contém todas as funções e a lógica do compressor (Código feito de forma artesanal).

Novas funcionalidades (Futuro):
-----------

- Parametrizar a entrada do número de processos via chamada no terminal (argv).
- Parametrizar o particionamento do tamanho do arquivo via chamada no terminal (argv).
- Parametrizar para que o programa descubra dinâmicamente os melhores tamanhos de tabela/palavra.
