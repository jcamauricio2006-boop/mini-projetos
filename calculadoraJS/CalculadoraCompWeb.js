//Esse foi um teste de estruturação do Fluxograma feito em um compilador da Web, chamado Programiz, não tem valores predefinidos e nem prompt para colocar valores fora
//variáveis
let valor1;
let valor2;
let OP;
let Resultado;
//classificação ou entrada de dados
valor1 = parseFloat(valor1);
valor2 = parseFloat(valor2);
OP = String(OP);
// Funções
function soma(valor1, valor2){
    return valor1+valor2;
}
function subtracao(valor1, valor2){
    return valor1-valor2;
}
function multiplicacao(valor1, valor2){
    return valor1*valor2;
}
function divisao(valor1, valor2){
    return valor1/valor2;
}
//Problemática
switch (OP){
    case "+":
        resultado = soma(valor1, valor2);
        break;
    case "-": 
        resultado = subtracao(valor1, valor2);
        break;
    case "x":
        resultado = multiplicacao(valor1, valor2);
        break;
    case "/": 
        resultado = divisao(valor1, valor2);
        break;
    default:
    resultado = "operação invalida"
}
// Saída
console.log(resultado)
