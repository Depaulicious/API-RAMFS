# Progetto Algoritmi e Principi dell'Informatica (AA 2016/17)

Implementazione in C di un filesystem in RAM.

[Specifiche](http://home.deib.polimi.it/pradella/IT/consegna.pdf).

## Come compilarlo

### Con cmake

```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Come file singolo

```shell
./build_singlefile.sh
```

Per generare il file singolo è necessario [c2singlefile](https://github.com/Depaulicious/c2singlefile).