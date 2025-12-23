import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import io from 'socket.io-client';

const API_URL = 'http://localhost';
const WS_URL = 'ws://localhost/ws';

function App() {
  const [userId, setUserId] = useState('user123');
  const [balance, setBalance] = useState(0);
  const [amount, setAmount] = useState('');
  const [description, setDescription] = useState('');
  const [orders, setOrders] = useState<any[]>([]);
  const [depositAmount, setDepositAmount] = useState('');

  useEffect(() => {
    loadOrders();
    loadBalance();
    
    const socket = io(WS_URL);
    
    socket.on('order_update', (data: any) => {
      toast.info(`Order ${data.order_id}: ${data.status} - ${data.message}`);
      loadOrders();
    });
    
    return () => {
      socket.disconnect();
    };
  }, []);

  const loadOrders = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/orders`, {
        params: { user_id: userId }
      });
      setOrders(response.data);
    } catch (error) {
      console.error('Failed to load orders:', error);
    }
  };

  const loadBalance = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/accounts/${userId}/balance`);
      setBalance(response.data.balance);
    } catch (error) {
      console.error('Failed to load balance:', error);
    }
  };

  const createOrder = async () => {
    try {
      await axios.post(`${API_URL}/api/orders`, {
        user_id: userId,
        amount: parseFloat(amount),
        description
      });
      toast.success('Order created successfully!');
      setAmount('');
      setDescription('');
      loadOrders();
    } catch (error) {
      toast.error('Failed to create order');
    }
  };

  const createAccount = async () => {
    try {
      await axios.post(`${API_URL}/api/accounts`, {
        user_id: userId
      });
      toast.success('Account created successfully!');
      loadBalance();
    } catch (error) {
      toast.error('Failed to create account');
    }
  };

  const deposit = async () => {
    try {
      await axios.post(`${API_URL}/api/accounts/${userId}/deposit`, {
        amount: parseFloat(depositAmount)
      });
      toast.success('Deposit successful!');
      setDepositAmount('');
      loadBalance();
    } catch (error) {
      toast.error('Failed to deposit');
    }
  };

  return (
    <div className="container">
      <ToastContainer />
      
      <h1>Интернет-магазин "Г ozон"</h1>
      
      <div className="section">
        <h2>Аккаунт</h2>
        <div className="form-group">
          <label>User ID:</label>
          <input 
            type="text" 
            value={userId} 
            onChange={(e) => setUserId(e.target.value)}
          />
        </div>
        <div className="balance">
          <strong>Баланс:</strong> {balance.toFixed(2)} руб.
        </div>
        <div className="buttons">
          <button onClick={createAccount}>Создать счет</button>
          <div className="deposit">
            <input 
              type="number" 
              placeholder="Сумма пополнения"
              value={depositAmount}
              onChange={(e) => setDepositAmount(e.target.value)}
            />
            <button onClick={deposit}>Пополнить</button>
          </div>
        </div>
      </div>
      
      <div className="section">
        <h2>Создать заказ</h2>
        <div className="form-group">
          <label>Сумма:</label>
          <input 
            type="number" 
            value={amount}
            onChange={(e) => setAmount(e.target.value)}
            placeholder="999.99"
          />
        </div>
        <div className="form-group">
          <label>Описание:</label>
          <input 
            type="text" 
            value={description}
            onChange={(e) => setDescription(e.target.value)}
            placeholder="Ноутбук"
          />
        </div>
        <button onClick={createOrder}>Создать заказ</button>
      </div>
      
      <div className="section">
        <h2>Мои заказы</h2>
        {orders.length === 0 ? (
          <p>Нет заказов</p>
        ) : (
          <table>
            <thead>
              <tr>
                <th>ID</th>
                <th>Сумма</th>
                <th>Описание</th>
                <th>Статус</th>
                <th>Дата</th>
              </tr>
            </thead>
            <tbody>
              {orders.map((order) => (
                <tr key={order.id}>
                  <td>{order.id.substring(0, 8)}...</td>
                  <td>{order.amount} руб.</td>
                  <td>{order.description}</td>
                  <td className={`status-${order.status.toLowerCase()}`}>
                    {order.status === 'NEW' && 'Новый'}
                    {order.status === 'FINISHED' && 'Оплачен'}
                    {order.status === 'CANCELLED' && 'Отменен'}
                  </td>
                  <td>{new Date(order.created_at * 1000).toLocaleString()}</td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
}

export default App;