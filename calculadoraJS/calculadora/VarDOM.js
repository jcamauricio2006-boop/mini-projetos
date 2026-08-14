// simplificação do codigo ===============================================================================================
function elementoID(b){
     return document.getElementById(b).addEventListener("click",function(){telaP.innerText+= b})
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
let menos = document.getElementById("menos").addEventListener("click", function(){telaP.innerHTML += "-"})
let mais = document.getElementById("soma").addEventListener("click", function(){telaP.innerHTML += "+"})
let vezes = document.getElementById("multiplicacao").addEventListener("click", function(){telaP.innerHTML += "*"})
let dividir = document.getElementById("divisao").addEventListener("click", function(){telaP.innerHTML += "/"})
// ========================================================================================================================
let resultado = document.getElementById("igualdade").addEventListener("click", function(){telaP.innerHTML = eval(telaP.textContent);})
// reset ==================================================================================================================
let reset = document.getElementById("reset").addEventListener("click",function(){telaP.innerHTML = " "})
