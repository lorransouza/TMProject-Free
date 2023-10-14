----
Para criar um novo NPCP, adicione o arquivo na pasta DataBase//Quests com a extensão "*.c"

O NPC deverá estar com a merchant padrão acima do nível 600. Para a quest 1, o nível deve ser 601, para a quest 10, o nível deverá ser 610.

///////////////

QUEST*    : Define o número da quest. 
	        O parâmetro vai logo ao lado
	        Limite atual: 15

Exemplo:
QUEST0 NAME Quest_Nome_Test

Parametros:

NAME	  : Não sei porque fiz isso, ainda não tem serventia 
POSITION  : Mesma coisa, já que é gerado no NPCGener, não faz sentido

CONDITION : Esta parte é a parte em que o NPC irá solicitar 
            itens ou atributos do jogador
            Você deve definir um index para cada uma, por exemplo

QUEST0 CONDITION-0
            No caso acima, o index definido seria o 0. O limite de pedidos atualmente é 10

	    SPEECH               -> Quando o que você está pedindo não é encontrado você pode enviar uma mensagem 
Exemplo:
QUEST0 CONDITION-0 ITEM 10 3330
QUEST0 CONDITION-0 SPEECH Você_precisa_de_10_trombetas.

        LEVEL "MIN" "MAX"    -> Você define o nível mínimo e máximo para utilizar o NPC
        ITEM "AMOUNT" "ITEM" -> Pede um item em uma quantidade definida por você
        EVOLUTION "VALUE"    -> Necessário uma evolução para usar o NPC - 0 = all, 1 = mortal, 2 = arch, 3 = cele, 4 = sub, 5 = mortal + arch, 6 = arch + cele, 7 = arch+cele+sub, 8=cele+sub
		GOLD "VALUE"         -> Solicita uma quantia de gold no inventário do usuário
		CLASS "VALUE" 	     -> Para usar o NPC deverá ter a classe indicada. 1 = tk, 2 = fm, 3 = bm, 4 = ht
		EQITEM "SLOT" "ITEM" -> Necessário ter um item XX equipado no slot YY
	

REWARD    : Esta é a parte em que você dá as premiações para o usuário.
            Você deve definir um index para cada uma, por exemplo:

QUEST0 REWARD-0

	No caso acima, o index definido seria o 0. O limite de entrega atualmente é 10

	SPEECH                -> Quando é entregue uma premiação, você pode por mensagens pelo NPC. 
	                         Poderá aparecer mais de uma no NPC, uma para cada premiação, mas vai floodar KK

	EXP "VALUE"						-> Entrega o valor de experiência para o personagem.
	LEVEL "VALUE"					-> Entrega XX níveis para o usuário, a experiência aumenta junto.
	GOLD "VALUE"					-> Entrega gold para o personagem.
	EQUIP "SLOT" "ITEM"				-> Equipa um item no personagem (ef1, efv1 inclusos).
	TELEPORT "X" "Y"				-> Teleporta o usuário para as coordenadas citadas
	REMOVEGOLD "VALUE"				-> Remove uma quantia de gold do personagem
	REMOVEEXP "VALUE"				-> Remove uma quantia de exp do personagem
	DELETEITEM "SLOT" "AMOUNT" "ID" -> Deleta a quantia de itens do personagem. O index seria de 0~5, podendo remover até 5 itens diferentes.
	EQDELETE "SLOT" "ITEM"			-> Exclui um item do equipamento do usuário, o ID do item tem que se o que estará equipado no momento.