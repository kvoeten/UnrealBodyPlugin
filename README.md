![alt text](https://github.com/kvoeten/UnrealBodyPlugin/blob/main/Resources/logo.png?raw=true)
# UnrealBodyPlugin
A standalone unreal engine plugin for simulating a full body with finger IK in virtual reality.

## Setup
1. Download the latest release, and unzip the files into your project's /plugins folder.
2. (Re)start the editor, and make sure the plugin is activated
3. To integrate the body with your VR Pawn, add a skeletal mesh and make sure it's animation class is set to the included ABP_IKBody.
4. To enable animation of the body, add the IKBody component to your pawn. The IKBodyComponent requires you to set a body reference, a camera reference and two controller references. This is best done on your construction script.
5. To enable finger IK you have to add hitboxes to the fingers. When you are satisfied with your hitbox locations on the skeleton, you use the 'Set All Hitboxes' function on the IKBody component to assign them during the construction script.
6. To start the finger IK you use the 'Start Finger IK' function on the IKBody component, this function takes an actor reference to check the overlap with. This reference should be the object you aim to interact with using finger ik.

## VRExpansion
The release page includes a 'Vive_PawnCharacter' from a vanilla VR Expansion sample project that has the IKBody plugin fully integrated. Assuming there haven't been any new releases of VR Expansion that make this pawn character incompatible, you can simply drop this pawn into your VR Exapansion project, replacing the original pawn.
