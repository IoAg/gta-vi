// Arff.h

#ifndef __ARFF_H__
#define __ARFF_H__

#include "ArffBase.h"

#include <chrono>

using namespace std;

// struct that holds the information about the change in one point
struct Change
{   
    Change() {row=0; column=0; value=0;
        time=chrono::system_clock::now();
    }
    Change(int r, int c, int val){row=r; column=c;
        value=val; time=chrono::system_clock::now();
    }
    int row;
    int column;
    double value;
    chrono::system_clock::time_point time;
};


class Arff : public ArffBase
{
public:
    Arff();

    Arff(const char* filename);

    void Save(const char *filename);

    void ChangeData(int row, int column, double value);
    ///< This function is called instead of the oveloaded [] operator in 
    ///< case that we require a reversible change.

    int WidthPx() const;

    void SetWidthPx(int width);

    int HeightPx() const;

    void SetHeightPx(int height);

    void UndoLastChange();

    void RedoLastChange();

    bool IsDataChanged();

private:
	vector<Change> m_changes;
    vector<Change> m_redoChanges;

    bool m_isChanged;

    void MoveChanges(vector<Change> &from, vector<Change> &to);
};

#endif /*__ARFF_H__*/
