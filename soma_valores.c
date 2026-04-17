#include <stdio.h>

int main() {
    float num1, num2, soma;

    // Entrada de dados
    printf("Digite o primeiro valor: ");
    scanf("%f", &num1);

    printf("Digite o segundo valor: ");
    scanf("%f", &num2);

    // Cálculo
    soma = num1 + num2;

    // Processamento e Saída
    printf("\nResultado da soma: %.2f\n", soma);

    if (soma >= 10) {
        printf("O valor e maior ou igual a 10.\n");
    } else {
        printf("O valor e menor que 10.\n");
    }

    return 0;
}