#pragma once

#include "Truss.h"

void    addNode( Truss* truss );
void    removeNode( Truss* truss );
void    switchMember( Truss* truss );
void    moveNode( Truss* truss );
void    thicken( Truss* truss );

Truss::Mutation*   selectMutation();