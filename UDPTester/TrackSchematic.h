#ifndef TRACKSCHEMATIC_H
#define TRACKSCHEMATIC_H

#include <QWidget>

class TrackSchematic : public QWidget
{
    Q_OBJECT
public:
    explicit TrackSchematic(QWidget *parent = 0);

    void addTrackSection(QWidget *section);

signals:

public slots:

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

private:

};

#endif // TRACKSCHEMATIC_H
