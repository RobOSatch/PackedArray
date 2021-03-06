#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>


// DO NOT CHANGE THE UNDERLYING TYPE!
// our MeshID has 32 bits. you can use those bits for whatever purpose you see fit.
// this is the only thing our user sees.
typedef uint32_t MeshID;


// DO NOT CHANGE!
// you can assume that we never store more than 256 entries.
static const unsigned int MAX_MESH_COUNT = 256;


// DO NOT CHANGE!
// your implementation needs to work with arbitrary types, whose layout we cannot change.
// you can assume that all types you're going to deal with are PODs, like the Mesh here.
struct Mesh
{
	// the mesh usually has several members (vertex buffer, index buffer, etc.).
	// for our exercise we only use a dummy member. this is only used to check whether the implementation works.
	int dummy;
};


// in our exercise, the RenderWorld only holds meshes, nothing more.
// the RenderWorld implementation makes sure that after *each* operation its internal data structure has no holes,
// and contains Mesh instances which are stored contiguously in memory.
class RenderWorld
{
public:
	RenderWorld(void)
		: m_meshCount(0), m_nextID(0)
	{
		for (int i = 0; i < MAX_MESH_COUNT; ++i)
		{
			m_accessArray[i] = i + 1;
			m_accessLookup[i] = -1;
		}
	}
		
	// Needs to be O(1)
	MeshID AddMesh(void)
	{
		// Update the next free meshID
		unsigned int meshID = m_nextID;
		m_nextID = m_accessArray[meshID];

		// Store the meshes position in the m_meshes array in the acessArray. Then reflect that in the accessLookup.
		m_accessArray[meshID] = m_meshCount;
		m_accessLookup[m_meshCount] = meshID;
		m_meshes[m_meshCount++] = Mesh();

		return meshID;
	}


	// Needs to be O(1)
	void RemoveMesh(MeshID id)
	{
		// Get the meshID of the mesh marked for deletion and then the accessID of the mesh which is going to shift to its place.
		unsigned int meshToDelete = m_accessArray[id];
		int shiftedMeshAccessID = m_accessLookup[m_meshCount - 1];

		// shift the last element into the deleteion gap. Then, mark the deleted mesh by resetting its dummy value.
		m_meshes[meshToDelete] = m_meshes[m_meshCount - 1];
		m_meshes[m_meshCount - 1].dummy = 0xcccccccc;

		// Update the accessArray and the accessLookup to represent the changes made by the deletion.
		m_accessArray[shiftedMeshAccessID] = meshToDelete;
		m_accessArray[id] = m_nextID;
		m_accessLookup[meshToDelete] = m_accessLookup[m_meshCount - 1];
		m_accessLookup[m_meshCount - 1] = -1;

		// Update the next id (LIFO queue)
		m_nextID = id;

		// decrease the count for the meshes in the scene
		m_meshCount--; 
	}


	// Needs to be O(1)
	Mesh* Lookup(MeshID id)
	{
		// First, check if the mesh at the specified ID has been deleted or never set and if so, return a nullptr.
		// Otherwise just return the mesh
		unsigned int innerID = m_accessArray[id];

		if (m_accessLookup[innerID] == -1)
		{
			return nullptr;
		}

		return &m_meshes[m_accessArray[id]];
	}


	// DO NOT CHANGE!
	// the implementation of this method needs to stay as it is.
	// you need to correctly implement all other methods to ensure that:
	// a) m_meshCount is up-to-date
	// b) m_meshes stores instances of Mesh contiguously, without any holes
	// c) external MeshIDs still refer to the correct instances somehow
	void Iterate(void)
	{
		for (unsigned int i = 0; i < m_meshCount; ++i)
		{
			printf("Mesh instance %d: dummy = %d\n", i, m_meshes[i].dummy);
		}
	}


private:
	MeshID m_nextID; // The next free meshID
	unsigned int m_accessArray[MAX_MESH_COUNT]; // An array, mapping the meshIDs to the innerIDs of the m_meshes array
	int m_accessLookup[MAX_MESH_COUNT]; // An array, mapping the innerIDs of the m_meshes array to the meshIDs.

	// DO NOT CHANGE!
	// these two members are here to stay. see comments regarding Iterate().
	Mesh m_meshes[MAX_MESH_COUNT];
	unsigned int m_meshCount;
};

struct ID
{
	MeshID _accessID;
	unsigned int _innerID;
};

int main(void)
{
	RenderWorld rw;

	// add 3 meshes to the world. we only ever refer to them by their ID, the RenderWorld has complete ownership
	// over the individual Mesh instances.
	MeshID meshID0 = rw.AddMesh();
	MeshID meshID1 = rw.AddMesh();
	MeshID meshID2 = rw.AddMesh();

	// lookup the meshes, and fill them with data.
	{
		Mesh* mesh0 = rw.Lookup(meshID0);
		mesh0->dummy = 0;
		Mesh* mesh1 = rw.Lookup(meshID1);
		mesh1->dummy = 1;
		Mesh* mesh2 = rw.Lookup(meshID2);
		mesh2->dummy = 2;
	}

	// by now, the world contains 3 meshes, filled with dummy data 0, 1 and 2.
	// in memory, the 3 meshes should be contiguous in memory:
	// [Mesh][Mesh][Mesh]
	rw.Iterate();

	// we now remove the second mesh (referenced by meshID1), which creates a hole in the world's data structure:
	// [Mesh][Empty][Mesh]
	// the world should internally update its data structure(s), so that the other two remaining meshes are stored contiguously in memory.
	rw.RemoveMesh(meshID1);

	// iteration must still work, because the instances are contiguous in memory.
	rw.Iterate();

	// even though the render world might have copied/changed some data structures, the two remaining meshes must still
	// refer to the correct object. this is verified by checking their dummy members.
	assert(rw.Lookup(meshID0)->dummy == 0);
	assert(rw.Lookup(meshID2)->dummy == 2);

	// the mesh referenced by meshID1 has been removed above, yet we intentionally try to access it.
	// the implementation should give an error, and return a nullptr in that case.
	Mesh* mesh1 = rw.Lookup(meshID1);
	assert(mesh1 == nullptr);

	return 0;
}
