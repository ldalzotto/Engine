A Ressource is an instanciated object that load data from the the asset database to allocate a system internal object.
The ressource object doesn't have any logic, it simply holds references to the allocated internal object.

# Architecture

<svg-inline src="ressource_architecture.svg"></svg-inline>

Every ressource is associated to an allocation unit. This allocation unit is responsible of handling all allocation and
deallocation inputs. If the criteria is met, it stores the event in a buffer that is later consumed by the main loop.

# Allocation/Deallocation

When an allocation is requested, the allocation unit generate a ressource token that is returned to the caller. Also, an
internal allocation event is generated. The consumer can retrieve the allocated ressource by providing the returned
token. When a deallocation is requested, the allocation unit generate a deallocation event. The token is invalid when
the event is consumed and that the ressource has been freed.

The same ressource can be allocated multiple times. When that happens, the same ressource token is returned and the
usage counter in incremented by one. Deallocation request make the counter decready by one. As soon as the counter
reaches zero, the deallocation event is generated.

# Allocation modes

The ressource allocation input can be provided by two ways:

** Requested to the database: **
If the ressource input is requested to the database, the caller provide the asset path used by the database to retrieve
the asset. <br/>

** Provided inline: **
If the ressource input is provided inline, the caller provide the same data as if it were retrieved by the database.

Allocation events are processed in the following order for a single allocation unit :

<svg-inline src="ressource_allocation_order.svg""></svg-inline>

# Ressource identification

All ressources are associated to an internal id. <br/>
When the ressource input is provided inline, the ressource id is given by the caller. <br/>
When the ressource input is provided by the database, the ressource id is computed by the hash of the asset path.

# Ressource dependencies

Ressources can be linked together (for example, a Mateial is linked to Shader) introducing an allocation and
deallocation execution order. A goal of the allocation units is to handle the recursive allocation/deallocation of
linked internal system objects, to that the internal systems can blindfully execute requests from the allocation unit.

If ressource A needs ressource B to work, then the allocation order will be : (B) -> (A). And the deallocation order : (
A) -> (B).

<svg-inline src="ressource_dependencies.svg"></svg-inline>

When the ressource allocation input is provided inline, it is up to the caller to provide input for the requested
ressource and it's dependencies. <br/>
When the ressource allocation input is provided by the database, then an additional asset database request is performed
to retrieve asset id dependencies.

<svg-inline src="ressource_allocation_database_dependencies.svg"></svg-inline>