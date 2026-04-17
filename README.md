# Compilador C para Assembly

Este README explica como o código em `soma_valores.c` é traduzido e executado em Assembly, usando as anotações do arquivo `Interpretando um compilador Assembly.txt`, usando a sintaxe do gcc em -masm=intel.

## 1. Visão geral do código C

O programa em `soma_valores.c` realiza o seguinte:

- declara três variáveis `float`: `num1`, `num2`, `soma`
- pede a entrada de dois valores pelo usuário usando `printf` e `scanf`
- soma os dois valores e guarda o resultado em `soma`
- exibe o resultado usando `printf`
- compara a soma com `10`
- imprime uma mensagem diferente se a soma for maior ou igual a 10 ou se for menor que 10
- retorna `0`

## 2. Preparação da pilha e do quadro de ativação

No começo do Assembly, o compilador monta o quadro de ativação da função `main`:

- `push rbp`: salva o valor antigo de `rbp` na pilha
- `mov rbp, rsp`: define `rbp` como o novo quadro de ativação
- `sub rsp, 16`: reserva 16 bytes de espaço na pilha para as variáveis locais

Esses passos são padrão para funções em x86-64 e garantem que o compilador tenha espaço para armazenar variáveis temporárias e locais.

## 3. Chamadas de entrada e saída (printf/scanf)

O Assembly traduz os `printf` e `scanf` do C para chamadas de função com argumentos em registradores:

- `mov edi, OFFSET FLAT:.LC0`
  - carrega o endereço da primeira string de prompt (`Digite o primeiro valor:`) em `edi`
- `mov eax, 0`
  - zera `eax` para indicar que não há argumentos em registradores SSE para `printf`
- `call printf`
  - invoca a função de impressão

Para `scanf`, o compilador prepara o endereço da variável onde o valor lido será armazenado:

- `lea rax, [rbp-8]`
  - calcula o endereço de `num1` na pilha
- `mov rsi, rax`
  - passa esse endereço como segundo argumento de `scanf`
- `mov edi, OFFSET FLAT:.LC1`
  - carrega o formato `"%f"` em `edi`
- `call __isoc99_scanf`
  - executa a leitura do float

O mesmo padrão se repete para `num2`, alterando o offset para `[rbp-12]`.

## 4. Cálculo da soma em ponto flutuante

A soma em C é feita com operações SSE em Assembly:

- `movss xmm1, DWORD PTR [rbp-8]`
  - carrega `num1` para o registrador `xmm1`
- `movss xmm0, DWORD PTR [rbp-12]`
  - carrega `num2` para `xmm0`
- `addss xmm0, xmm1`
  - soma `xmm1` a `xmm0`
- `movss DWORD PTR [rbp-4], xmm0`
  - armazena o resultado em `soma` no espaço reservado na pilha

Isso mostra que o compilador traduz diretamente a operação `soma = num1 + num2;` em instruções de ponto flutuante.

## 5. Preparação para imprimir o resultado

Antes de chamar `printf` para exibir `soma`, o Assembly faz conversão e limpeza:

- `pxor xmm2, xmm2`
  - zera `xmm2` para evitar lixo de memória e manter performance
- `cvtss2sd xmm2, DWORD PTR [rbp-4]`
  - converte `soma` de `float` para `double`, porque `printf` com `%.2f` espera `double` via chamada variádica
- `movq rax, xmm2`
  - move o valor convertido para `rax`
- `movq xmm0, rax`
  - move o valor para `xmm0`, registrador usado para o primeiro argumento em chamadas de função SSE
- `mov edi, OFFSET FLAT:.LC3`
  - carrega a string de formato "Resultado da soma: %.2f\n"
- `call printf`
  - imprime o resultado

Esta sequência mostra como o compilador ajusta tipos e passagem de argumentos para atender à ABI do sistema.

## 6. Comparação com 10 e fluxo condicional

A instrução `if (soma >= 10)` se torna uma comparação de ponto flutuante seguida de salto condicional:

- `movss xmm0, DWORD PTR [rbp-4]`
  - carrega `soma`
- `comiss xmm0, DWORD PTR .LC4[rip]`
  - compara `soma` com o valor `10.0` armazenado em `.LC4`
- `jb .L7`
  - salta para o bloco do `else` se `soma` for menor que 10

A seguir, o Assembly escolhe qual mensagem imprimir:

- Caso `soma >= 10`:
  - `mov edi, OFFSET FLAT:.LC5`
  - chama `puts` para imprimir "O valor e maior ou igual a 10."
- Caso contrário:
  - o salto `.L7` leva para o bloco do `else`
  - a mensagem do `else` é impressa

O compilador transforma o `if` em comparações e saltos, mantendo a lógica de decisão do C.

## 7. Conclusão: o que acontece por baixo dos panos

A partir do código C:

- o compilador gera instruções para montar o quadro de função (`rbp`, `rsp`)
- converte chamadas de biblioteca (`printf`, `scanf`) em chamadas com argumentos passados por registrador
- traduz operações de ponto flutuante para instruções SSE (`movss`, `addss`)
- faz conversões de tipo necessárias para a ABI do `printf`
- implementa controle de fluxo com comparações e saltos condicionais

Esse processo é o núcleo do trabalho do compilador: mapear operações de alto nível em instruções de baixo nível que o processador pode executar, ao mesmo tempo em que preserva os tipos, o layout de memória e a lógica de controle do programa.
