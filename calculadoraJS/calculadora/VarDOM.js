// simplificação do codigo ===============================================================================================
function elementoID(b){
     return document.getElementById(b).addEventListener("click",function(){telaP.innerText+= b})
}
function elementoIDOP(a, b){
    return  document.getElementById(a).addEventListener("click",function(){telaP.innerText+= b})
}
// ======================================================================================================================
const telaP = document.getElementById("telap")
let Num1 = elementoID("1")
let Num2 = elementoID("2")
let Num3 = elementoID("3")
let Num4 = elementoID("4")
let Num5 = elementoID("5")
let Num6 = elementoID("6")
let Num7 = elementoID("7")
let Num8 = elementoID("8")
let Num9 = elementoID("9")
let Num0 = elementoID("0")
// =======================================================================================================================
let menos = elementoIDOP("menos", "-")
let mais = elementoIDOP("soma", "+")
let vezes = elementoIDOP("multiplicacao", "*")
let dividir = elementoIDOP("divisao", "/")
// ========================================================================================================================
let resultado = document.getElementById("igualdade").addEventListener("click", function(){telaP.innerHTML = eval(telaP.textContent);})
// reset ==================================================================================================================
let reset = document.getElementById("reset").addEventListener("click",function(){telaP.innerHTML = " "})
