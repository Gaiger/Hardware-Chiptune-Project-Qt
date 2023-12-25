#ifndef WAVECHARTVIEW_H
#define WAVECHARTVIEW_H

#include <QChartView>
using namespace QtCharts;

class WaveChartView : public QChartView
{
	Q_OBJECT
public:
	WaveChartView(QChart *chart, QWidget *parent = nullptr);

	void GiveWave(QByteArray wave_bytearray);
	void CleanUndrawnWave(void);
private :
	QByteArray m_remain_wave_bytearray;
};

#endif // WAVECHARTVIEW_H
