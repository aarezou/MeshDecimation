/*
	Sample code by Wallace Lira <http://www.sfu.ca/~wpintoli/> based on
	the four Nanogui examples and also on the sample code provided in
		  https://github.com/darrenmothersele/nanogui-test
	 
	All rights reserved. Use of this source code is governed by a
	BSD-style license that can be found in the LICENSE.txt file.
*/
 
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/glcanvas.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
 
// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
 
 
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif
 
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
 
#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif
 
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;
using std::map;
using std::make_pair;
 
using nanogui::Screen;
using nanogui::Window;
using nanogui::GroupLayout;
using nanogui::Button;
using nanogui::CheckBox;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::MatrixXu;
using nanogui::MatrixXf;
using nanogui::Label;
using nanogui::Arcball;
 
 
class Edge;
class Vertex {
	public:
	Eigen::Vector3f val, norm;
	Eigen::Matrix4f quad;
	int ind;
	Edge *edge;
};
class Face {
	public:
	Edge *edge;
	int ind;
};
class Edge {
	public:
	static map<pair<int, int>, Edge *> incompleteEdges;
	int ind;
	Vertex *fromV, *toV;
  	Face *lFace, *rFace;
  	Edge *preCCW, *nexCCW, *preCW, *nexCW, *sym;
	void setParams(Vertex *from, Vertex *to, int indFrom, int indTo, Edge *p, Edge *n, Face *f)
	{
		this->fromV = from;
		this->toV = to;
		this->preCCW = p;
		this->nexCCW = n;
		this->lFace = f;
		//cout << "before vertex if" << endl;
		if (this->fromV->edge == NULL)
			this->fromV->edge = this;
		//cout << "after vertex if" << endl;
		if (incompleteEdges.find(make_pair(indTo, indFrom)) != incompleteEdges.end())
		{	
			Edge *symE = incompleteEdges[make_pair(indTo, indFrom)];
			this->sym = symE;
			this->preCW = symE->nexCCW;
			this->nexCW = symE->preCCW;
			this->rFace = symE->lFace;
			symE->sym = this;
			sym->preCW = this->nexCCW;
			sym->nexCW = this->preCCW;
			sym->rFace = this->lFace;
			incompleteEdges.erase(make_pair(indTo, indFrom));
		}
		else
			incompleteEdges[make_pair(indFrom, indTo)] = this;
		//cout << "after map if" << endl;
	}
	/*bool operator < (Edge *x)const {
		if (this->fromV->ind < x->fromV->ind)
			return true;
		if (this->fromV->ind == x->fromV->ind && this->toV->ind < x->toV->ind)
			return true;
		return false;
	}*/
};
map<pair<int, int>, Edge *> Edge::incompleteEdges;

class MyGLCanvas : public nanogui::GLCanvas {
public:
	
	MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent) {
		using namespace nanogui;
 
		mShader.initFromFiles("a_smooth_shader", "StandardShading.vertexshader", "StandardShading.fragmentshader");
	
		// After binding the shader to the current context we can send data to opengl that will be handled
		// by the vertex shader and then by the fragment shader, in that order.
		// if you want to know more about modern opengl pipeline take a look at this link
		// https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview
		mShader.bind();
	
			//mShader.uploadIndices(indices);
		mShader.uploadAttrib("vertexPosition_modelspace", positions);
		mShader.uploadAttrib("color", colors);
		mShader.uploadAttrib("vertexNormal_modelspace", normals);
	
		// ViewMatrixID
		// change your rotation to work on the camera instead of rotating the entire world with the MVP matrix
		Matrix4f V;
		V.setIdentity();
		//V = lookAt(Vector3f(0,12,0), Vector3f(0,0,0), Vector3f(0,1,0));
		mShader.setUniform("V", V);
	
		//ModelMatrixID
		Matrix4f M;
		M.setIdentity();
		mShader.setUniform("M", M);
		
		// This the light origin position in your environment, which is totally arbitrary
		// however it is better if it is behind the observer
		mShader.setUniform("LightPosition_worldspace", Vector3f(-2,6,-4));
 
	}
 
	//flush data on call
	~MyGLCanvas() {
		mShader.free();
	}
 
	//Method to update the rotation on each axis
	void setRotation(nanogui::Vector3f vRotation) {
		mRotation = vRotation;
	}
	void setRotationAxis(float vRotation, int pos) {
		mRotation[pos] = vRotation;
	}
	void setTranslation(float vTranslation, int pos) {
		mTranslation[pos] = vTranslation;
	}
	void setZoom(float vZoom) {
		mZoom = vZoom;  
	}
	void setShade(bool vShade) {
		shade = vShade;
	}
	void setWire(bool vWire) {
		wire = vWire;
	}
	void setFlat(bool vFlat) {
		flatShade = vFlat;
		computeNormals();
	}
	
	void loadObj(string path)
	{
		mx = 0.0;
		mn = INFF;
		string tmp;
		std::ifstream fin (path);
		fin >> tmp >> vertNum >> faceNum;
		edgeNum = faceNum*3;
		string str;
		
		int vertInd = 0;
		int faceInd = 0;
		int edgeInd = 0;
		while(getline(fin, str)) 
    	{	 
			std::istringstream sin(str);
			sin >> tmp;
			if (tmp == "v")
			{
				verts[vertInd] = new Vertex();
				verts[vertInd]->edge = NULL;
				verts[vertInd]->ind = vertInd;
				sin >> verts[vertInd]->val[0]>> verts[vertInd]->val[1] >> verts[vertInd]->val[2];
				for (int i=0; i<3; i++)
				{
					mx = std::max (mx, verts[vertInd]->val[i]);
					mn = std::min (mn, verts[vertInd]->val[i]);
				}
				vertInd ++;
			}
			else if (tmp == "f")
			{
				int ind[3];
				Edge *e[3]; 
				for (int i=0; i<3; i++)
				{
					sin >> ind[i];
					ind[i]--;
					e[i] = new Edge();
					edges[edgeInd] = e[i];
					edges[edgeInd]->ind = edgeInd;
					edgeInd ++;
				}	
				
				faces[faceInd] = new Face();
				faces[faceInd]->edge = e[0];
				faces[faceInd]->ind = faceInd;

				e[0]->setParams(verts[ind[0]], verts[ind[1]], ind[0], ind[1], e[2], e[1], faces[faceInd]);
				e[1]->setParams(verts[ind[1]], verts[ind[2]], ind[1], ind[2], e[0], e[2], faces[faceInd]);
				e[2]->setParams(verts[ind[2]], verts[ind[0]], ind[2], ind[0], e[1], e[0], faces[faceInd]);
				faceInd ++;
				
			}
		}
		fin.close();
		for (int j=0; j<vertNum; j++) setVertexNorm(verts[j]);
		doEverything();
		cout <<"loaded obj" << endl;
	}
	void saveObj(string path)
	{
		cout << path << endl;
		std::ofstream fout(path);
		fout << "# " << vertNum << " " << faceNum << endl;
		for (int i=0; i<vertNum; i++)
			fout << "v " << verts[i]->val[0] << " " << verts[i]->val[1]<< " "<< verts[i]->val[2] << " "<< endl;
		for (int i=0; i<faceNum; i++)
		{
			fout << "f ";
			Edge *startE = faces[i]->edge, *currentE = startE;
			do
			{
				fout << currentE->fromV->ind + 1 << " ";
				currentE = currentE->nexCCW;
			} while (currentE != startE);
			fout << endl;
		}
		//for (int i=0; i<faceNum; i++)
		//	fout <<"f " << ind[i][0]+1 << " " << ind[i][1]+1 << " " << ind[i][2]+1 << endl;
		fout.close();
	}
	void mergeEdges(Edge * e1, Edge * e2)
	{
		e1->fromV->edge = e1;
		e1->lFace = e2->rFace;
		e1->preCCW = e2->sym->preCCW;
		e1->nexCCW = e2->sym->nexCCW;
		e1->preCCW->nexCCW = e1;
		e1->nexCCW->preCCW = e1;
		e1->lFace->edge = e1;
		
		e2->fromV->edge = e2;
		e2->lFace = e1->rFace;
		e2->preCCW = e1->sym->preCCW;
		e2->nexCCW = e1->sym->nexCCW;
		e2->preCCW->nexCCW = e2;
		e2->nexCCW->preCCW = e2;
		e2->lFace->edge = e2;

		edges[e1->sym->ind] = edges[edgeNum-1];
		edges[e1->sym->ind]->ind = e1->sym->ind;
		edges[edgeNum-1] = NULL;
		edgeNum --;
		
		edges[e2->sym->ind] = edges[edgeNum-1];
		edges[e2->sym->ind]->ind = e2->sym->ind;	
		edges[edgeNum-1] = NULL;
		edgeNum --;

		delete(e1->sym);
		delete(e2->sym);

		e1->sym = e2;
		e2->sym = e1;
	}
	void deleteVertex(Vertex *old, Vertex *nw, Edge * e)
	{
		Face * startF = old->edge->lFace, * currentF = startF;
		Edge * startE, * currentE, * ret=NULL;
		do
		{
			startE = currentF->edge;
			currentE = startE;
			while (currentE->toV != old) currentE = currentE->nexCCW;
			currentE->toV = nw;
			currentE->sym->fromV = nw;
			currentF = currentE->rFace;
		} while (currentF != startF);
		delete(old);
	}
	int findNeighs(Vertex * v, int index)
	{
		int num = 0;
		Face * startF = v->edge->lFace, * currentF = startF;
		Edge * startE, * currentE, * ret=NULL;
		do
		{
			startE = currentF->edge;
			currentE = startE;
			while (currentE->toV != v) currentE = currentE->nexCCW;
			adj[index][num++] = currentE->fromV->ind;
			
			currentF = currentE->rFace;
		} while (currentF != startF);
		return num;
	}
	int numCommonNeighs(Vertex * v1, Vertex * v2)
	{
		int num1 = findNeighs(v1, 0), num2 = findNeighs(v2, 1), common = 0;
		for (int i=0; i<num1; i++)
			for (int j=0; j<num2; j++)
				if (adj[0][i] == adj[1][j])
					common ++;
		return common;
	}
	bool collapseEdge(Edge *e, Eigen::Vector3f val)
	{
		Vertex *v = e->fromV;
		
		// If v1 and v2 have more than 2 common neighbors, e should not be collapsed. 
		if (numCommonNeighs(e->fromV, e->toV) > 2) return false;
		
		// Setting values of new vertex
		v->val = val;
		v->norm = (e->fromV->norm + e->toV->norm).normalized();//setVertexNorm(v);//
		v->quad = e->fromV->quad + e->toV->quad;

		// Removing v2 from the vertex array
		verts[e->toV->ind] = verts[vertNum-1];
		verts[e->toV->ind]->ind = e->toV->ind; 
		verts[vertNum-1] = NULL;
		vertNum --;
	
		// Updating neighbors of v2 to conect to v1 instead
		deleteVertex(e->toV, v, e);

		// Merging the remaining 2 edges of each face that e is a part of
		mergeEdges(e->nexCCW, e->nexCCW->nexCCW);
		mergeEdges(e->sym->nexCCW, e->sym->nexCCW->nexCCW);
		
		// Deleting faces that e is a part of from the face array 
		faces[e->lFace->ind] = faces[faceNum-1];
		faces[e->lFace->ind]->ind = e->lFace->ind;
		faces[faceNum-1] = NULL;
		faceNum--;
		
		faces[e->rFace->ind] = faces[faceNum-1];
		faces[e->rFace->ind]->ind = e->rFace->ind;
		faces[faceNum-1] = NULL;
		faceNum--;

		delete(e->rFace);
		delete(e->lFace);
		
		// Deleting e and its symetric edge from the edge array
		edges[e->ind] = edges[edgeNum-1];
		edges[e->ind]->ind = e->ind;
		edges[edgeNum-1] = NULL;
		edgeNum --;
		
		edges[e->sym->ind] = edges[edgeNum-1];
		edges[e->sym->ind]->ind = e->sym->ind;
		edges[edgeNum-1] = NULL;
		edgeNum --;
		
		delete(e);
		delete(e->sym);

		return true;
	}
	float err(Vertex * v1, Vertex * v2, Eigen::Vector3f pos)
	{
		Eigen::Matrix4f quad = v1->quad + v2->quad;
		Eigen::Vector4f tmp ;
		tmp << pos, 1.0;
		return tmp.transpose()*quad*tmp;
	}
	Eigen::Vector3f minPosition(Vertex * v1, Vertex * v2)
	{
		// This function will return the optimal position for a new vertex resulting from combining v1 and v2 
		Eigen::Matrix4f quad = v1->quad + v2->quad;
		Eigen::Matrix4f tmp;
		Eigen::Vector4f tmpRes, rhs;
		Eigen::Vector3f res, alt;
		rhs << 0, 0, 0, 1;
		tmp.row(0) << quad.row(0);
		tmp.row(1) << quad.row(1);
		tmp.row(2) << quad.row(2);
		tmp.row(3) << 0, 0, 0, 1;
		tmpRes = tmp.inverse() * rhs;
		res << tmpRes(0), tmpRes(1), tmpRes(2); 
		alt = (v1->val + v2->val)/2;
		
		if (abs(err(v1, v2, res)) <= abs(err(v1, v2, alt))) return res;
		//cout << "alt: " << endl;
		return alt;
	}
	void applyMeshDec()
	{
		for (int j=0; j<decNum; j++)
		{
		//	cout << "collapsing " << j << endl;
			if (edgeNum <= 12)
				break;
			int minInd = 0;
			Edge * minCandid = edges[minInd];
			Eigen::Vector3f minPos = minPosition(minCandid->fromV, minCandid->toV);
			float minErr = err(minCandid->fromV, minCandid->toV, minPos);
			
			for (int i=0; i<candidateNum; i++)
			{
				int ind = rand() % edgeNum;
				Edge * newCandid = edges[ind];
				Eigen::Vector3f newPos = minPosition(newCandid->fromV, newCandid->toV);
				float newErr = err(newCandid->fromV, newCandid->toV, newPos);
				if (newErr < minErr)
				{
					minInd = ind;
					minErr = newErr;
					minPos = newPos;
					minCandid = newCandid;
				}
			}
			if (!collapseEdge(minCandid, minPos)) j--;
		}
		doEverything();
	//	cout << "done with meshDec" << endl;
	}
	Eigen::Vector3f getFaceNorm(Face *f)
	{
		//cout << "face" << endl;
		Eigen::Vector3f vec0 = f->edge->toV->val - f->edge->fromV->val;
		Eigen::Vector3f vec1 = f->edge->nexCCW->toV->val - f->edge->nexCCW->fromV->val;
		return (vec1.cross(vec0)).normalized();
	}
	Eigen::Matrix4f getQuad(Eigen::Vector3f val, Eigen::Vector3f norm)
	{
		float d = -(val.dot(norm));
		Eigen::Vector4f tmp;
		tmp << norm, d;
		return tmp*tmp.transpose();
	}
	void setVertexNorm(Vertex *v)
	{
		//cout << v->ind + 1 << endl;
		Face * startF = v->edge->lFace, * currentF = startF;
		Edge * startE, * currentE;
		int numF = 0;
		Eigen::Vector3f norm, tmpNorm;
		Eigen::Matrix4f quad, tmpQuad;
		quad.setZero();
		norm << 0.0, 0.0, 0.0;
		do
		{
			tmpNorm = getFaceNorm(currentF);
			tmpQuad = getQuad(v->val, tmpNorm);
			norm = norm + tmpNorm;
			quad = quad + tmpQuad;
			numF ++;
			startE = currentF->edge;
			currentE = startE;
			while (currentE->toV != v) currentE = currentE->nexCCW;
			currentF = currentE->rFace;
		} while (currentF != startF);
		norm = norm/numF;
		v->norm = norm.normalized();
		v->quad = quad;
	}
	
	void computeNormals()
	{
	int total = 0;
		for (int i=0; i<faceNum; i++)
		{
			int num = 0;
			Edge *startE = faces[i]->edge, *currentE = startE;
			do
			{	
				if (flatShade) normals.col(total + num) << getFaceNorm(faces[i]);
				else normals.col(total + num) << currentE->fromV->norm;
				colors.col(total + num) = normals.col(total + num);
				currentE = currentE->nexCCW;
				num ++;
			} while (currentE != startE);
			total = total + num;
		}
	//	cout << "done with normals" << endl;
	}
	void computePositions()
	{
		int total = 0;
		float tp = (mx+mn)/2, bt = (mx-mn)/2;
		for (int i=0; i<faceNum; i++)
		{
			int num = 0;
			Edge *startE = faces[i]->edge, *currentE = startE;
			do
			{
				positions.col(total + num) << ((currentE->fromV->val) - Eigen::Vector3f(tp, tp, tp) )*2/bt;
				//colors.col(total + num) << 0.5, 0.0, 0.5;
				currentE = currentE->nexCCW;
				num ++;
			} while (currentE != startE);
			total = total + num;
		}
	//	cout << "done with positions" << endl;
	}
	void doEverything()
	{
		computePositions();
		computeNormals();
	}
	//Method to update the mesh itself, can change the size of it dynamically, as shown later
	void updateMeshPositions(MatrixXf newPositions){
		positions = newPositions;
 
	}
 
	//OpenGL calls this method constantly to update the screen.
	virtual void drawGL() override {
		using namespace nanogui;
 
		//refer to the previous explanation of mShader.bind();
		mShader.bind();
 
		//this simple command updates the positions matrix. You need to do the same for color and indices matrices too
		mShader.uploadAttrib("vertexPosition_modelspace", positions);
		mShader.uploadAttrib("vertexNormal_modelspace", normals);
		mShader.uploadAttrib("color", colors);
 
		//This is a way to perform a simple rotation using a 4x4 rotation matrix represented by rmat
		//mvp stands for ModelViewProjection matrix
		Matrix4f rot_trans, mvp, scale;

		scale.setIdentity();
		rot_trans.setIdentity();        
		mvp.setIdentity();
		
		scale= Eigen::Scaling(Eigen::Vector4f(mZoom, mZoom, mZoom, 1.0f));
		rot_trans.block<3,1>(0,3) = mTranslation; 
		rot_trans.topLeftCorner<3,3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mRotation[0], Vector3f::UnitX()) *
														Eigen::AngleAxisf(mRotation[1], Vector3f::UnitY()) *
														Eigen::AngleAxisf(mRotation[2], Vector3f::UnitZ())) * 0.25f;
			
		mvp = rot_trans * scale * mvp;
		mShader.setUniform("MVP", mvp);
 
		// If enabled, does depth comparisons and update the depth buffer.
		// Avoid changing if you are unsure of what this means.
		/* Draw 12 triangles starting at index 0 of your indices matrix */
		/* Try changing the first input with GL_LINES, this will be useful in the assignment */
		/* Take a look at this link to better understand OpenGL primitives */
		/* https://www.khronos.org/opengl/wiki/Primitive */
 
		glEnable(GL_DEPTH_TEST);
		if (shade)
		{
			mShader.setUniform("wire", 0);	
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonOffset(-1,-1);
			mShader.drawArray(GL_TRIANGLES, 0, faceNum*3);	
		}	
		if (wire)	
		{	
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			mShader.setUniform("wire", 1);
				mShader.drawArray(GL_TRIANGLES, 0, faceNum*3);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}        
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_DEPTH_TEST);
	}
	int getEdgeNum()
	{
		return edgeNum;
	}
	int getDecNum()
	{
		return decNum;
	}
	void setDecNum(int val)
	{
		decNum = val;
	}
	int getCandNum()
	{
		return candidateNum;
	}
	void setCandidateNum(int val)
	{
		candidateNum = val;
	}
//Instantiation of the variables that can be acessed outside of this class to interact with the interface
//Need to be updated if a interface element is interacting with something that is inside the scope of MyGLCanvas
private:
	static const int MAXF = 50000 + 100;
	static constexpr float INFF = 50000;
	int vertNum, faceNum, edgeNum;
	MatrixXf positions = MatrixXf(3, 3*MAXF);
	MatrixXf normals = MatrixXf(3, 3*MAXF);
	MatrixXf colors = MatrixXf(3, 3*MAXF);
	nanogui::GLShader mShader;
	Eigen::Vector3f mRotation;
	Eigen::Vector3f mTranslation;
	bool shade = true, wire = false, flatShade = true;
	float mZoom = 1.0;
	Vertex * verts[MAXF];
	Face * faces[MAXF];
	Edge * edges[MAXF*3];
	float mx = 0.0, mn = INFF;
	int decNum = 0, candidateNum = 0, adj[2][MAXF];
};
 
 
class ExampleApplication : public nanogui::Screen {
public:
	ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1200, 900), "NanoGUI Cube and Menus", false) {
		using namespace nanogui;
 
	//OpenGL canvas demonstration
 
	//First, we need to create a window context in which we will render both the interface and OpenGL canvas
		Window *window = new Window(this, "GLCanvas Demo");
		window->setPosition(Vector2i(15, 15));
		window->setLayout(new GroupLayout());
	 
	//OpenGL canvas initialization, we can control the background color and also its size
		mCanvas = new MyGLCanvas(window);
		mCanvas->setBackgroundColor({100, 100, 100, 255});
		mCanvas->setSize({800, 800});
 
		//widgets demonstration
		nanogui::GLShader mShader;
 
	//Then, we can create another window and insert other widgets into it
	Window *anotherWindow = new Window(this, "Basic widgets");
		anotherWindow->setPosition(Vector2i(850, 15));
		anotherWindow->setLayout(new GroupLayout());
 
	// Demonstrates how a button called "New Mesh" can update the positions matrix.
	// This is just a demonstration, you actually need to bind mesh updates with the open file interface
 
	//this is how we write captions on the window, if you do not want to write inside a button  
	Widget *sliders = new Widget(anotherWindow);
	sliders->setLayout(new BoxLayout(Orientation::Horizontal,
									   Alignment::Middle, 0, 0));
	
	Widget *panelRot = new Widget(sliders);
		panelRot->setLayout(new BoxLayout(Orientation::Vertical,
									   Alignment::Middle, 0, 0));
	panelRot->setPosition(nanogui::Vector2i(0,0));

	//Demonstration of rotation along one axis of the mesh using a simple slider, you can have three of these, one for each dimension
	
	new Label(panelRot, "Rotation on First Axis", "sans-bold");
	Slider *rotSlider1 = new Slider(panelRot);
		rotSlider1->setValue(0.5f);
		rotSlider1->setFixedWidth(150);
		
	new Label(panelRot, "Rotation on Second Axis", "sans-bold");
	Slider *rotSlider2 = new Slider(panelRot);
		rotSlider2->setValue(0.5f);
		rotSlider2->setFixedWidth(150);
	
	new Label(panelRot, "Rotation on Third Axis", "sans-bold");
	Slider *rotSlider3 = new Slider(panelRot);
		rotSlider3->setValue(0.5f);
		rotSlider3->setFixedWidth(150);

	rotSlider1->setCallback([&](float value) {
		float radians = (value - 0.5f)*2*M_PI;
		mCanvas->setRotationAxis(radians, 0);
	});
	rotSlider2->setCallback([&](float value) {
		float radians  = (value - 0.5f)*2*M_PI;
		mCanvas->setRotationAxis(radians, 1);		
	});
	rotSlider3->setCallback([&](float value) {
		float radians  = (value - 0.5f)*2*M_PI;
		mCanvas->setRotationAxis(radians, 2);
	});

	
	Widget *panelTrans = new Widget(sliders);
		panelTrans->setLayout(new BoxLayout(Orientation::Vertical,
									   Alignment::Middle, 0, 0));
	panelTrans->setPosition(nanogui::Vector2i(0,1));

	new Label(panelTrans, "Horizontal Translation", "sans-bold");
	Slider *transSlider0 = new Slider(panelTrans);
		transSlider0->setValue(0.5f);
		transSlider0->setFixedWidth(150);
		transSlider0->setCallback([&](float value) {
			float amount = (value - 0.5f) * 2;
			mCanvas->setTranslation(amount, 0);
		});
	new Label(panelTrans, "Vertical Translation", "sans-bold");
	Slider *transSlider1 = new Slider(panelTrans);
		transSlider1->setValue(0.5f);
		transSlider1->setFixedWidth(150);
		transSlider1->setCallback([&](float value) {
			float amount = (value - 0.5f) * 2;
			mCanvas->setTranslation(amount, 1);
		});
	new Label(panelTrans, "Zoom", "sans-bold");
	Slider *zoomSlider = new Slider(panelTrans);
		zoomSlider->setValue(0.5f);
		zoomSlider->setFixedWidth(150);
		zoomSlider->setCallback([&](float value) {
			float amount = (value*2);
			mCanvas->setZoom(amount);
		});
	new Label(anotherWindow, "Check box", "sans-bold");
		CheckBox *cb = new CheckBox(anotherWindow, "Show Shading",
			[](bool state) { }
		);
	cb->setCallback([&](bool state) {
		mCanvas->setShade(state); 
	});
	cb->setChecked(true);
	cb = new CheckBox(anotherWindow, "Show Wireframe",
			[](bool state) {}
		);
	cb->setCallback([&](bool state) {
		mCanvas->setWire(state); 
	});
	cb = new CheckBox(anotherWindow, "Flat Shading",
			[](bool state) {}
		);
	cb->setCallback([&](bool state) {
		mCanvas->setFlat(state); 
	});
	cb->setChecked(true);
	new Label(anotherWindow, "Number of Edges to Decimate", "sans-bold");
	static TextBox *textBox = new TextBox(anotherWindow);
		textBox->setValue("0");
		textBox->setEditable(true);
		textBox->setCallback([&](const std::string &value) {
        //    std::cout << "Current Automatic Value: " << value << std::endl;
			mCanvas->setDecNum(std::stoi(value));
            return true;
        });
	new Label(anotherWindow, "Number of Candidates to Choose each Edge from", "sans-bold");
	textBox = new TextBox(anotherWindow);
		textBox->setValue("0");
		textBox->setEditable(true);
		textBox->setCallback([&](const std::string &value) {
        //    std::cout << "Current Automatic Value: " << value << std::endl;
			mCanvas->setCandidateNum(std::stoi(value));
            return true;
        });
		
	Button *b;
		b = new Button(anotherWindow, "Decimate");
		b->setCallback([&] (){
			int val = mCanvas->getEdgeNum(), value = mCanvas->getDecNum(), candNum = mCanvas->getCandNum();
			if (val - value * 6 < 12)
				auto dlg = new MessageDialog(this, MessageDialog::Type::Information, "Title", "Mesh does not have enough edges. Please choose a smaller number.");
			else if (candNum <= 0)
				auto dlg = new MessageDialog(this, MessageDialog::Type::Information, "Title", "Number of candidates can't be less than one. Please choose a bigger number.");
			else
				mCanvas->applyMeshDec();
		});
	new Label(anotherWindow, "File dialog", "sans-bold");
		b = new Button(anotherWindow, "Open");
		b->setCallback([&] {
			string res  = file_dialog({ {"obj", "Object File"}, {"txt", "Text file"} }, false);
			mCanvas->loadObj(res);
			cout << "File dialog result: " << res << endl;
		});
		b = new Button(anotherWindow, "Save");
		b->setCallback([&] {
			string res =  file_dialog({ {"obj", "Object File"}, {"txt", "Text file"} }, true);
			mCanvas->saveObj(res);
		});
	//Message dialog demonstration, it should be pretty straightforward
		b = new Button(anotherWindow, "Close Application");
		b->setCallback([&] {
		auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "Are you sure you want to close the application?", "Yes", "No", true);
		dlg->setCallback([](int result) 
		{ 
			if (result == 0)
				nanogui::shutdown();
		});
		});
	//Method to assemble the interface defined before it is called
		performLayout();
	}
 
	//This is how you capture mouse events in the screen. If you want to implement the arcball instead of using
	//sliders, then you need to map the right click drag motions to suitable rotation matrices
	virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override {
		if (button == GLFW_MOUSE_BUTTON_3 ) {
		//Get right click drag mouse event, print x and y coordinates only if right button pressed
		cout << p.x() << "     " << p.y() << "\n";
			return true;
		}
		return false;
	}
 
	virtual void drawContents() override {
		// ... put your rotation code here if you use dragging the mouse, updating either your model points, the mvp matrix or the V matrix, depending on the approach used
	}
 
	virtual void draw(NVGcontext *ctx) {
	/* Animate the scrollbar */
		//mProgress->setValue(std::fmod((float) glfwGetTime() / 10, 1.0f));
 
		/* Draw the user interface */
		Screen::draw(ctx);
	}
 
 
private:
	MyGLCanvas *mCanvas;
};
 
int main(int /* argc */, char ** /* argv */) {
	
	try {
		nanogui::init();
 
			/* scoped variables */ {
			nanogui::ref<ExampleApplication> app = new ExampleApplication();
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop();
		}
 
		nanogui::shutdown();
	} catch (const std::runtime_error &e) {
		std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
		#if defined(_WIN32)
			MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
		#else
			std::cerr << error_msg << endl;
		#endif
		return -1;
	}
 
	return 0;
}
