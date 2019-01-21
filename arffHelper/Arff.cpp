// Arff.cpp

#include "Arff.h"

using namespace std;

#define MAX_TIME_GAP_MS 200

Arff::Arff() : ArffBase(), m_isChanged(false)
{
}

Arff::Arff(const char* filename) : ArffBase(filename), m_isChanged(false)
{
}

void Arff::Save(const char *filename) 
{
	m_isChanged = false;

    ArffBase::Save(filename);
}

void Arff::ChangeData(int row, int column, double value)
{
    // store change
    Change change;
    change.row = row;
    change.column = column;
    change.value = (*this)[row][column];
    m_changes.push_back(change);

    // apply change
    (*this)[row][column] = value;

    // clear prevously applied changes
    m_redoChanges.clear();

    // set state
    m_isChanged = true;
}

int Arff::WidthPx()
{
    string value;
    if (GetMetadata("width_px", value))
        return stoi(value);
    else
        return -1;
}

void Arff::SetWidthPx(int width)
{
    SetMetadata("width_px", to_string(width));
}

int Arff::HeightPx()
{
    string value;
    if (GetMetadata("height_px", value))
        return stoi(value);
    else
        return -1;
}

void Arff::SetHeightPx(int height)
{
    SetMetadata("height_px", to_string(height));
}

void Arff::UndoLastChange()
{
    MoveChanges(m_changes, m_redoChanges);
    m_isChanged = true;
}

void Arff::RedoLastChange()
{
    MoveChanges(m_redoChanges, m_changes);
    m_isChanged = true;
}

bool Arff::IsDataChanged()
{
    return m_isChanged;
}

// PRIVATE:

void Arff::MoveChanges(vector<Change> &from, vector<Change> &to)
{
    if (from.empty())
        return;

    chrono::system_clock::time_point prevTime = from.back().time;
    chrono::system_clock::time_point curTime;
    unsigned int timeDiffMs;
    Change change;

    while(!from.empty())
    {
        change = from.back();
        curTime = change.time;
        timeDiffMs = abs(chrono::duration_cast<chrono::milliseconds>(curTime - prevTime).count());

        if (timeDiffMs > MAX_TIME_GAP_MS)
            break;

        double tmpValue = (*this)[change.row][change.column];
        (*this)[change.row][change.column] = change.value;
        change.value = tmpValue;

        to.push_back(change);
        from.pop_back();
    }
}
