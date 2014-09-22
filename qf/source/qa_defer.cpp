/// \file
/// \brief QP::QActive::defer() and QP::QActive::recall() definitions.
/// \ingroup qf
/// \cond
///***************************************************************************
/// Product: QF/C++
/// Last updated for version 5.3.1
/// Last updated on  2014-09-05
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// \endcond

#define QP_IMPL           // this is QP implementation
#include "qf_port.h"      // QF port
#include "qf_pkg.h"
#include "qassert.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qa_defer")

//****************************************************************************
/// \description
/// This function is part of the event deferral support. An active object
/// uses this function to defer an event \c e to the QF-supported native
/// event queue \c eq. QF correctly accounts for another outstanding
/// reference to the event and will not recycle the event at the end of
/// the RTC step. Later, the active object might recall one event at a
/// time from the event queue.
///
/// \arguments
/// \arg[in] \c eq  pointer to a "raw" thread-safe queue to recall
///                 an event from.
/// \arg[in] \c e   pointer to the event to be deferred
///
/// \returns 'true' (success) when the event could be deferred and 'false'
/// (failure) if event deferral failed due to overflowing the queue.
///
/// An active object can use multiple event queues to defer events of
/// different kinds.
///
/// \sa QP::QActive::recall(), QP::QEQueue
bool QActive::defer(QEQueue * const eq, QEvt const * const e) const {
    return eq->post(e, static_cast<uint_fast16_t>(1)); // non-asserting post
}

//****************************************************************************
/// \description
/// This function is part of the event deferral support. An active object
/// uses this function to recall a deferred event from a given QF
/// event queue. Recalling an event means that it is removed from the
/// deferred event queue \c eq and posted (LIFO) to the event queue of
/// the active object.
///
/// \arguments
/// \arg[in]  \c eq  pointer to a "raw" thread-safe queue to recall
///                  an event from.
///
/// \returns 'true' if an event has been recalled and 'false' if not.
///
/// \note An active object can use multiple event queues to defer events of
/// different kinds.
///
/// \sa QP::QActive::recall(), QP::QEQueue, QP::QActive::postLIFO_()
bool QActive::recall(QEQueue * const eq) {
    QEvt const * const e = eq->get(); // try to get evt from deferred queue
    bool const recalled = (e != null_evt); // event available?
    if (recalled) {
        this->postLIFO(e); // post it to the _front_ of the AO's queue

        QF_CRIT_STAT_
        QF_CRIT_ENTRY_();

        // is it a dynamic event?
        if (e->poolId_ != u8_0) {

            // after posting to the AO's queue the event must be referenced
            // at least twice: once in the deferred event queue (eq->get()
            // did NOT decrement the reference counter) and once in the
            // AO's event queue.
            Q_ASSERT_ID(210, e->refCtr_ > u8_1);

            // we need to decrement the reference counter once, to account
            // for removing the event from the deferred event queue.
            QF_EVT_REF_CTR_DEC_(e); // decrement the reference counter
        }

        QF_CRIT_EXIT_();
    }
    return recalled; // event not recalled
}

} // namespace QP


