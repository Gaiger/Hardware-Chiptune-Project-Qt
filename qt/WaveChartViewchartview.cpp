#include <QtCharts>
#include <QDebug>
#include "WaveChartView.h"

#define FRAME_SAMPLINGS					(180 * 5)

WaveChartView::WaveChartView(QChart *p_chart, QWidget *parent)
	: QChartView(p_chart, parent)
{
	p_chart->setBackgroundVisible(false);
	p_chart->legend()->hide();

		//p_chart->setTitle("Simple line chart example");
	p_chart->layout()->setContentsMargins(0, 0, 0, 0);
	p_chart->setMargins(QMargins(0,0,0,0));
	p_chart->setBackgroundRoundness(0);

	QValueAxis *p_axis_x = new QValueAxis();
	p_axis_x->setRange(0, FRAME_SAMPLINGS);
	p_chart->addAxis(p_axis_x, Qt::AlignBottom);

	QValueAxis *p_axis_y = new QValueAxis();
	p_axis_y->setRange(-160, 160);
	p_chart->addAxis(p_axis_y, Qt::AlignLeft);

	QLineSeries *p_series = new QLineSeries();
	p_chart->addSeries(p_series);
	p_series->attachAxis(p_axis_x);
	p_series->attachAxis(p_axis_y);

	QPen pen(10);
	pen.setColor(QColor(Qt::darkGray).lighter(115));
	p_series->setPen(pen);

	p_chart->axes(Qt::Horizontal).first()->setRange(0, FRAME_SAMPLINGS);
	p_chart->axes(Qt::Vertical).first()->setRange(-10, 256 + 10);
	p_chart->axes(Qt::Horizontal).at(0)->setVisible(false);
	p_chart->axes(Qt::Vertical).at(0)->setVisible(false);

	QChartView::setRenderHint(QPainter::Antialiasing);
	WaveChartView::Reset();
}

/**********************************************************************************/

void WaveChartView::GiveWave(QByteArray wave_bytearray)
{
	m_remain_wave_bytearray += wave_bytearray;

	do
	{
		if(FRAME_SAMPLINGS > m_remain_wave_bytearray.size()){
			break;
		}

		while( 2 * FRAME_SAMPLINGS < m_remain_wave_bytearray.size() )
		{
			m_remain_wave_bytearray.remove(0, FRAME_SAMPLINGS);
			continue;
		}

		QList<QPointF> points_vector;
		for(int i = 0; i < FRAME_SAMPLINGS ; i++){
			uint8_t value = (uint8_t)m_remain_wave_bytearray.at(i);
			points_vector.append( QPointF((double)i, (double)value) );
		}

		QXYSeries *p_series = (QXYSeries*)QChartView::chart()->series().first();
		p_series->replace(points_vector);
		m_remain_wave_bytearray.remove(0, FRAME_SAMPLINGS);
	}while(0);
}

/**********************************************************************************/

void WaveChartView::CleanUndrawnWave(void)
{
	m_remain_wave_bytearray.clear();
}

/**********************************************************************************/

void WaveChartView::Reset(void)
{
	CleanUndrawnWave();
	QList<QPointF> points_vector;
	for(int i = 0; i < FRAME_SAMPLINGS ; i++){
		points_vector.append( QPointF((double)i, (double)128) );
	}

	QXYSeries *p_series = (QXYSeries*)QChartView::chart()->series().first();
	p_series->replace(points_vector);
}
