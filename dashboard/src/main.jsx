import React from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import ParkingDashboard from './ParkingDashboard'

const App = () => (
  <ParkingDashboard />
)

createRoot(document.getElementById('root')).render(<App />)
