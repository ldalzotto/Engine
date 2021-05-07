A Resource is an instantiated object that load data from the asset database to allocate a system internal object.
The resource object doesn't have any logic, it simply holds references to the allocated internal object.

# Architecture

<svg-inline src="resource_architecture.svg"></svg-inline>

Every resource is associated to an allocation unit. This allocation unit is in charge of handling all allocation and
deallocation inputs. If the criteria is met, it stores the event in a buffer that is later consumed by the main loop.

# Allocation/Deallocation

When an allocation is requested, the allocation unit generate a resource token that is returned to the caller. Also, an
internal allocation event is generated. The consumer can retrieve the allocated resource by providing the returned
token. When a deallocation is requested, the allocation unit generate a deallocation event. The token is invalid when
the event is consumed and that the resource has been freed.

The same resource can be allocated multiple times. When that happens, the same resource token is returned and the
usage counter in incremented by one. Deallocation request make the counter decrease by one. As soon as the counter
reaches zero, the deallocation event is generated.

# Allocation modes

The resource allocation input can be provided by two ways:

** Requested to the database: **
If the resource input is requested to the database, the caller provide the asset path used by the database to retrieve
the asset. <br/>

** Provided inline: **
If the resource input is provided inline, the caller provide the same data as if it were retrieved by the database.

Allocation events are processed in the following order for a single allocation unit :

<svg-inline src="resource_allocation_order.svg""></svg-inline>

# Resource identification

All resources are associated to an internal ID. <br/>
When the resource input is provided inline, the resource ID is given by the caller. <br/>
When the resource input is provided by the database, the resource ID is computed by the hash of the asset path.

# Resource dependencies

Resources can be linked together (for example, a Material is linked to Shader) introducing an allocation and
deallocation execution order. A goal of the allocation units is to handle the recursive allocation/deallocation of
linked internal system objects, so that the internal systems can blindly execute requests from the allocation unit.

If resource A needs resource B to work, then the allocation order will be : (B) -> (A). And the deallocation order : (
A) -> (B).

<svg-inline src="resource_dependencies.svg"></svg-inline>

When the resource allocation input is provided inline, it is up to the caller to provide input for the requested
resource and it's dependencies. <br/>
When the resource allocation input is provided by the database, then an additional asset database request is performed
to retrieve asset ID dependencies.

<svg-inline src="resource_allocation_database_dependencies.svg"></svg-inline>