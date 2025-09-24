## Problem
kernel defines the abi for the implementation ->
implementation depends on the abi             ->
abi then includes the implementation          ->
circular dependency

This worked for simple cases, but using the same ABI in more than one file quickly proves
that this pattern is very fragile.

ABI shoud not know a thing about implementation.

The hal folder needs a rework.

