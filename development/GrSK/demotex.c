/*
 * GrSK first module tester
 *
 * Rough idea of what GrSK can do. This program display a texture map
 * that rotates around its center. The code is inherited from a 1994
 * demo program that I've written for a totally different context,
 * with all its bells and bugs. So don't complain... 
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <grsk.h>
#include <teststubs.h>

#include "demotex.h"

WORD16 destY;
char *TexMapBits;
WORD16 TexMapWidth;
char *virtual_screen;

UWORD16 maxVert;
UWORD16 numVerts;
POINT *VertexPtr;
POINT *TexVertsPtr;

LOCAL BOOLEAN stepEdge(INOUT EDGE *edge)

/*
 * Compute next source and destination pixels, and change current 
 * active edge if it is needed.
 *
 * INPUT:
 * <parameter name1> <explanation>
 * <parameter name2> <explanation>
 * ...
 *
 * RETURNS:
 * A boolean that tells if another edge has to be processed or not.
 */

{
	if (--edge->remainingScans == 0) {

		/* Non ci sono pi— scanline, se i lati sono finiti esce. */

		if (setUpEdge(edge, edge->currentEnd) == 0) {
			return(FALSE);
		}

		return(TRUE);
	}

	/* Select next source pixel */

	edge->sourceY += edge->sourceStepY;
	edge->sourceX += edge->sourceStepX;

	/* Select next destination pixel. Only X-coord since Y is the big for loop index*/

	edge->destX += edge->destXIntStep;

	/* Compute relative error, and if big enough do appropriate rounding */

	if ((edge->destXErrTerm += edge->destXAdjUp) > 0) {
	    edge->destX += edge->destXDirection;
	    edge->destXErrTerm -= edge->destXAdjDown;
	}

	return(TRUE);
}

LOCAL BOOLEAN setUpEdge(INOUT EDGE * edge, INPUT WORD16 startVert)

/* Per ogni lato ricalcola i coefficienti per la ricerca del pixel
   nella texture map e nel poligono fisico.
   Ritorna TRUE se la ricerca ha avuto successo (nel lato corrente
   c'Š almeno una scanline da processare) FALSE se non ci sono pi—
   lati da controllare.						   */

{
	WORD16 nextVert;
	WORD16 destXWidth;
	FIXEDPOINT destYHeight;

	for (;;) {
		if (startVert == maxVert) {
			return(FALSE);
		}

		/* Avanza al prossimo lato. Il poligono Š una forma geometrica
	  	 chiusa, quindi dopo l'ultimo vertice c'Š il primo.           */

		nextVert = startVert + edge->direction;
		if (nextVert >= numVerts) {
			nextVert = 0;
		} else if (nextVert < 0) {
			nextVert = numVerts - 1;
		}

		/* Se Š rimasta una scanline nell'edge corrente ricalcola le
		   variabili necessarie per il calcolo della posizione del pixel
		   sorgente e destinazione.					*/

		if ((edge->remainingScans =
	  	  VertexPtr[nextVert].Y - VertexPtr[startVert].Y) != 0) {
			destYHeight = INT_TO_FIXED(edge->RemainingScans);
			edge->currentEnd = nextVert;
			edge->sourceX = INT_TO_FIXED(TexVertsPtr[startVert].X);
			edge->sourceY = INT_TO_FIXED(TexVertsPtr[startVert].Y);
			edge->sourceStepX = FixedDiv(INT_TO_FIXED(TexVertsPtr[nextVert].X) -
			                   edge->sourceX, destYHeight);
			edge->sourceStepY = FixedDiv(INT_TO_FIXED(TexVertsPtr[nextVert].Y) -
			                    edge->sourceY, destYHeight);

			if ((destXWidth =
		      (VertexPtr[nextVert].X - VertexPtr[startVert].X)) <= 0) {

		  	/* Vuol dire che il lato parte da destra e va a sinistra... */

				edge->destXDirection = -1;
				destXWidth = -destXWidth;
				edge->destXIntStep = -(destXWidth / edge->remainingScans);
			} else {

				/* ...o viceversa.  */

				edge->destXDirection = 1;
				edge->destXIntStep = destXWidth / edge->remainingScans;
			}

			edge->destX = VertexPtr[startVert].X;

			/* Se la dimensione della scanline destinazione Š diversa dalla
		     dimensione della scanline della texture map, occorre scrivere
		     un numero di pixel variabile della texture map sul poligono.
		     Dato che tutti i rapporti delle due dimensioni sono possibili,
		     occorre tenere in conto i casi dove DimensioneTexture/Dimensione
		     Poligono non Š un numero intero, calcolando l'errore assoluto. */

			edge->destXErrTerm = 1 - edge->remainingScans;
			edge->destXAdjUp = destXWidth % edge->remainingScans;
			edge->destXAdjDown = edge->remainingScans;

			return(TRUE);
		}

		startVert = nextVert;
	}
}

LOCAL VOID drawScanLine(INPUT EDGE *leftEdge, INPUT EDGE *rightEdge)

{
	UBYTE *vramPtr;
	UWORD16 index;
	FIXEDPOINT sourceX=leftEdge->sourceX;
	FIXEDPOINT sourceY=leftEdge->sourceY;
	FIXEDPOINT sourceXStep;
	FIXEDPOINT sourceYStep;
	FIXEDPOINT destWidth;
	UWORD16 destX=leftEdge->destX;
	UWORD16 destXMax=rightEdge->destX;

	if(destXMax-destX<=0)
		return;

	destWidth=INT_TO_FIXED(destXMax-destX);
	sourceXStep=FixedDiv(rightEdge->sourceX-sourceX,destWidth);
	sourceYStep=FixedDiv(rightEdge->sourceY-sourceY,destWidth);

	vramPtr=lmdriver->vramBaseAddress;
	vramPtr+=(destY*)lmdriver->displyWidth;
	vramPtr+=destX;

	for(;destX<destXMax;destX++) {
		index=(sourceY>>16)*texMapWidth;
		index+=(sourceX>>16);
		*vramPtr++=textureMap[index];
		sourceX+=sourceXStep;
		sourceY+=sourceYStep;
	}
}

LOCAL VOID setupTextureMap(INPUT TEXTUREMAP *texMap)

{
	texMapWidth=texMap->width;
	textureMap=texMap->bits;
}

LOCAL VOID drawTextureMappedPoly(POLY *polygon, TEXTUREMAP *texMap)

/* Disegna un poligono di vertici Polygon contenente la bitmap contenuta
   in TexMap. Vi Š una corrispondenza biunivoca tra i pixel del poligono
   e quelli della bitmap, rispettando l'orientazione spaziale della
   bitmap (se il poligono ruota, la bitmap ruota anch'essa).              */

{
	WORD16 minY;
	WORD16 maxY;
	WORD16 minVert;
	WORD16 i;
	EDGE rightEdge;
	EDGE leftEdge;
	
	numVerts = polygon->numVertex;
	VertexPtr = polygon->point;
	
	/* I vertici del poligono sono gi… ordinati in senso orario, basta
	   trovare il vertice con Y minore e maggiore per ottenere le scanline
	   da riempire.							  */

	minY = 32767;
	maxY = -32768;

	for (i=0; i<numVerts; i++) {
		if (VertexPtr[i].Y < MinY) {
			minY = VertexPtr[i].Y;
			minVert = i;
		}
		if (VertexPtr[i].Y > MaxY) {
			maxY = VertexPtr[i].Y;
			maxVert = i;
		}
	}

	/* Scarta poligoni degeneri */

	if (minY >= maxY) {
		return;
	}

	destY = minY;

  setupTexture(texMap);
  
	/* Setta i valori iniziali del loop di scansione.
	   Ad ogni Y vengono settate le variabili per il calcolo del pixel
	   di destinazione, in una modalit… analoga all'algoritmo che disegna
	   i vettori.							  */

	leftEdge.direction = -1;
	setUpEdge(&leftEdge, minVert);
	rightEdge.direction = 1;
	setUpEdge(&rightEdge, minVert);

	/* Loop di riempimento.
	   Il texture mapping ha molte affinit… con la routine di riempimento
	   di poligoni, nel senso che ogni pixel interno alla figura deve essere
	   mappato all'interno della bitmap che compone il texture map.
	   Per semplicit… la bitmap della texture viene considerata un poligono,
	   e viene fatta una scansione del poligono sorgente (texture) e
	   destinazione (poligono texture mappato)  con lo stesso sistema.      */

	for (;;) {

		/* Disegna la scanline clippando sulla Y */

 		if(destY>lmdriver->displayHeight)
			return;

		if(destY>=0) {
			drawScanLine(&leftEdge, &rightEdge);
		}

		/* Calcola le intersezioni della scanline con i lati
		   del poligono destinazione.				*/

		if (!stepEdge(&leftEdge)) {
			break;
		}
		if (!stepEdge(&rightEdge)) {
			break;
		}

		/* La prossima scanline Š sempre al pixel della riga successiva */

    destY++;
	}
}

extern TEXTUREMAP MyTexture;

POINT logpoints[]= { { -50,-50 },{ 50,-50 },{ 50,50 },{ -50,50 } };

POINT pointlist[4];
POLY mypoly= { 4, pointlist } ;

int main()

{
	WORD16 angle=0;

	/* Setup graphic mode */

	GrSKsetGraphicMode();
	GrSKsetPalette(texturePalette);
	virtual_screen=KLmalloc(64000);
	
	setupTextureMap(&MyTexture);

	/* Display loop */

	for(;;angle+=4) {

		/* Clear the display buffer */

		clearVirtualScreen();

		/* Do animation on graphic object */

		rotate(logpoints, angle, pointlist);

		/* Apply texture map to object */

		drawTextureMappedPoly(&mypoly, &MyTexture);

		/* Display the buffer to physical device */

		copyVirtualScreenToVGA();

		if(kbhit()) {
			UWORD16 key;

			key=getch();

			switch(key) {
				case 27:
					set_alpha_mode();
					KLfree(virtual_screen);
					exit(0);
					break;
				case '+':
					logpoints[0].X-=10;
					logpoints[0].Y-=10;
					logpoints[1].X+=10;
					logpoints[1].Y-=10;
					logpoints[2].X+=10;
					logpoints[2].Y+=10;
					logpoints[3].X-=10;
					logpoints[3].Y+=10;
					break;
				case '-':
					logpoints[0].X+=10;
					logpoints[0].Y+=10;
					logpoints[1].X-=10;
					logpoints[1].Y+=10;
					logpoints[2].X-=10;
					logpoints[2].Y-=10;
					logpoints[3].X+=10;
					logpoints[3].Y-=10;
					break;
			}
		}
	}
	
	exit(0);
}
